/**
 * @file upload_manager_impl.hpp
 * @author Herbert Jones
 * @brief Private implementation for the uplaod manager
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <deque>

#include "../upload_modification.hpp"
#include "../upload_request.hpp"
#include "../upload_status.hpp"

#include "upload_state_machine.hpp"

#include "mediafire_sdk/api/user/get_action_token.hpp"
#include "mediafire_sdk/utils/mutex.hpp"

// Forward declarations
namespace boost { namespace filesystem { class path; } }
namespace mf { namespace api { class SessionMaintainer; } }
// END forward declarations

namespace mf {
namespace uploader {
namespace detail {

typedef std::function<void(::mf::uploader::UploadStatus)> StatusCallback;

/**
 * @class UploadManagerImpl
 * @brief Manage a set of uploads.
 */
class UploadManagerImpl :
    public UploadStateMachineCallbackInterface,
    public std::enable_shared_from_this<UploadManagerImpl>
{
public:
    UploadManagerImpl(
            ::mf::api::SessionMaintainer * session_maintainer
        );
    virtual ~UploadManagerImpl();

    UploadHandle Add(
            const UploadRequest & request,
            StatusCallback callback
        );
    void ModifyUpload(
            UploadHandle upload_handle,
            ::mf::uploader::UploadModification modification
        );

private:
    ::mf::api::SessionMaintainer * session_maintainer_;
    boost::asio::io_service * io_service_;

    std::deque<StateMachinePointer> to_hash_;
    std::deque<StateMachinePointer> to_upload_;

    std::set<std::string> uploading_hashes_;

    std::set<StateMachinePointer> requests_;

    std::string action_token_;
    std::chrono::time_point<std::chrono::steady_clock> action_token_expires_;
    enum class ActionTokenState
    {
        Invalid,
        Retrieving,
        Error,
        Valid
    } action_token_state_;
    boost::asio::steady_timer action_token_retry_timer_;

    void ActionTokenRetry(
            const boost::system::error_code & err
        );

    uint32_t max_concurrent_hashings_;
    uint32_t max_concurrent_uploads_;

    std::set<UploadStateMachine*> enqueued_to_start_hashings_;
    std::set<UploadStateMachine*> enqueued_to_start_uploads_;
    uint32_t current_hashings_;
    uint32_t current_uploads_;

    bool disable_enqueue_;

    mf::utils::mutex mutex_;

    void Tick();
    void EnqueueTick();
    void Tick_StartUploads();
    void Tick_StartHashings();

    void HandleActionToken(
            const mf::api::user::get_action_token::Response & response
        );

    // -- UploadStateMachineCallbackInterface ----------------------------------
    virtual void HandleAddToHash(StateMachinePointer) override;
    virtual void HandleRemoveToHash(StateMachinePointer) override;

    virtual void HandleAddToUpload(StateMachinePointer) override;
    virtual void HandleRemoveToUpload(StateMachinePointer) override;

    virtual void HandleComplete(StateMachinePointer) override;

    virtual void IncrementHashingCount(StateMachinePointer) override;
    virtual void DecrementHashingCount(StateMachinePointer) override;

    virtual void IncrementUploadingCount(StateMachinePointer) override;
    virtual void DecrementUploadingCount(StateMachinePointer) override;
    // -- END UploadStateMachineCallbackInterface ------------------------------
};

}  // namespace detail
}  // namespace uploader
}  // namespace mf
