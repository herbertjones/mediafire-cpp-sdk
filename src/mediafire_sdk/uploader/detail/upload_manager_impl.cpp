/**
 * @file upload_manager_impl.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "upload_manager_impl.hpp"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/variant/apply_visitor.hpp"

#include "mediafire_sdk/uploader/error.hpp"

namespace action_token = mf::api::user::get_action_token;
namespace posix_time = boost::posix_time;
using sclock = std::chrono::steady_clock;

namespace {
const std::chrono::seconds kActionTokenRetry(15);
// Default lifetime is 1440 minutes(24h).  We should get a new upload token
// before it expires.
const std::chrono::minutes kActionTokenLife(1440/4*3);
}  // namespace

namespace mf {
namespace uploader {
namespace detail {

UploadManagerImpl::UploadManagerImpl(
        ::mf::api::SessionMaintainer * session_maintainer
    ) :
    session_maintainer_(session_maintainer),
    io_service_(session_maintainer->HttpConfig()->GetWorkIoService()),
    action_token_state_(ActionTokenState::Invalid),
    action_token_retry_timer_(*io_service_),
    max_concurrent_hashings_(2),
    max_concurrent_uploads_(2),
    current_hashings_(0),
    current_uploads_(0),
    disable_enqueue_(false)
{
}

UploadManagerImpl::~UploadManagerImpl()
{
    disable_enqueue_ = true;

    for (auto & request : requests_)
    {
        request->Disconnect();
        request->process_event(event::Error{
            make_error_code(mf::uploader::errc::Cancelled),
            "Cancelled due to shutdown."
            });
    }
    requests_.clear();
}

void UploadManagerImpl::Add(
        const UploadRequest & upload_request,
        StatusCallback callback
    )
{
    UploadConfig config;

    config.session_maintainer = session_maintainer_;
    config.filepath = upload_request.local_file_path_;
    config.on_duplicate_action = upload_request.on_duplicate_action_;
    config.callback_interface =
        static_cast<UploadStateMachineCallbackInterface*>(this);
    config.status_callback = callback;
    config.cloud_file_name = upload_request.utf8_target_name_;
    config.target_folder = upload_request.upload_target_folder_;

    auto request = std::make_shared<UploadStateMachine>(std::move(config));

    requests_.insert(request);

    // Initialization step
    request->process_event(event::Start{});

    Tick();
}

void UploadManagerImpl::ModifyUpload(
        const boost::filesystem::path & filepath,
        ::mf::uploader::UploadModification upload_modification
    )
{
    class Visitor : public boost::static_visitor<>
    {
    public:
        Visitor(
                UploadManagerImpl * um,
                const boost::filesystem::path & filepath
            ) :
            this_(um), filepath_(filepath)
        {}

        void operator()(modification::Cancel) const
        {
            for (auto & request : this_->requests_)
            {
                if (request->Path() == filepath_)
                {
                    request->ProcessEvent(event::Error{
                        make_error_code(mf::uploader::errc::Cancelled),
                        "Cancellation requested"
                        });
                    return;
                }
            }
        }

        void operator()(modification::Pause) const
        {
            for (auto & request : this_->requests_)
            {
                if (request->Path() == filepath_)
                {
                    request->ProcessEvent(event::Error{
                        make_error_code(mf::uploader::errc::Paused),
                        "Pause requested."
                        });
                    return;
                }
            }
        }

    private:
        UploadManagerImpl * this_;
        const boost::filesystem::path & filepath_;
    };

    boost::apply_visitor(Visitor(this, filepath), upload_modification);
}

void UploadManagerImpl::Tick()
{
    Tick_StartUploads();

    Tick_StartHashings();
}

void UploadManagerImpl::EnqueueTick()
{
    if ( ! disable_enqueue_ )
    {
        io_service_->post( boost::bind( &UploadManagerImpl::Tick,
                shared_from_this()) );
    }
}

void UploadManagerImpl::Tick_StartHashings()
{
    auto current_count = current_hashings_;

    if ( ! to_hash_.empty()
        && current_count < max_concurrent_hashings_ )
    {
        ++current_count;

        auto request = to_hash_.front();
        to_hash_.pop_front();

        request->process_event(event::StartHash{});
    }
}

void UploadManagerImpl::Tick_StartUploads()
{
    auto current_count = current_uploads_;

    if ( ! to_upload_.empty() )
    {
        const auto now = sclock::now();
        if ( action_token_state_ != ActionTokenState::Valid
            || action_token_expires_ < now )
        {
            // Invalid token

            if ( action_token_state_ == ActionTokenState::Error
                && now < action_token_expires_ )
            {
                // Skip till timeout.
            }
            else
            {
                action_token_state_ = ActionTokenState::Retrieving;

                auto self = shared_from_this();
                session_maintainer_->Call(
                    action_token::Request(action_token::Type::Upload),
                    [self](action_token::Response response)
                    {
                        self->HandleActionToken(response);
                    }
                    );
            }
            // If retrieving or error and not reached retry timeout, skip.
        }
        else if ( current_count < max_concurrent_uploads_ )
        {
            ++current_count;

            auto request = to_upload_.front();
            to_upload_.pop_front();
            assert( ! action_token_.empty() );
            request->process_event(event::StartUpload{action_token_});
        }
    }
}

void UploadManagerImpl::HandleActionToken(
        const mf::api::user::get_action_token::Response & response
    )
{
    if ( response.error_code )
    {
        // Retry soon if unable to obtain token.
        action_token_state_ = ActionTokenState::Error;

        action_token_expires_ = sclock::now() + kActionTokenRetry;
        action_token_retry_timer_.expires_at(action_token_expires_);

        action_token_retry_timer_.async_wait(
            boost::bind(
                &UploadManagerImpl::ActionTokenRetry,
                shared_from_this(),
                boost::asio::placeholders::error
            )
        );
    }
    else
    {
        action_token_state_ = ActionTokenState::Valid;
        action_token_ = response.action_token;
        action_token_expires_ = sclock::now() + kActionTokenLife;
        Tick();
    }
}

void UploadManagerImpl::ActionTokenRetry(
        const boost::system::error_code & err
    )
{
    if (!err && action_token_state_ == ActionTokenState::Error)
    {
        action_token_.clear();
        action_token_state_ = ActionTokenState::Invalid;
        Tick();
    }
}

// -- UploadStateMachineCallbackInterface --------------------------------------
void UploadManagerImpl::HandleAddToHash(StateMachinePointer request)
{
    to_hash_.push_back(request);

    EnqueueTick();
}
void UploadManagerImpl::HandleRemoveToHash(StateMachinePointer request)
{
    auto it = std::find(to_hash_.begin(), to_hash_.end(), request);
    if (it != to_hash_.end())
        to_hash_.erase(it);

    EnqueueTick();
}

void UploadManagerImpl::HandleAddToUpload(StateMachinePointer request)
{
    to_upload_.push_back(request);

    EnqueueTick();
}
void UploadManagerImpl::HandleRemoveToUpload(StateMachinePointer request)
{
    auto it = std::find(to_upload_.begin(), to_upload_.end(),
        request);
    if (it != to_upload_.end())
        to_upload_.erase(it);

    EnqueueTick();
}

void UploadManagerImpl::HandleComplete(StateMachinePointer request)
{
    requests_.erase(request);

    EnqueueTick();
}

void UploadManagerImpl::IncrementHashingCount(StateMachinePointer)
{
    ++current_hashings_;
}

void UploadManagerImpl::DecrementHashingCount(StateMachinePointer)
{
    --current_hashings_;
}

void UploadManagerImpl::IncrementUploadingCount(StateMachinePointer)
{
    ++current_uploads_;
}

void UploadManagerImpl::DecrementUploadingCount(StateMachinePointer)
{
    --current_uploads_;
}
// -- END UploadStateMachineCallbackInterface ----------------------------------

}  // namespace detail
}  // namespace uploader
}  // namespace mf
