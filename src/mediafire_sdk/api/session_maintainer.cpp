/**
 * @file session_maintainer.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 *
 * Beware, mutex lock being used.
 */
#include "session_maintainer.hpp"

#include <algorithm>
#include <deque>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "boost/variant/apply_visitor.hpp"
#include "boost/variant/get.hpp"

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/utils/mutex.hpp"

#if ! defined(NDEBUG)
// Counter to keep track of number of HttpRequests
boost::atomic<int> mf::api::detail::session_maintainer_request_count(0);
#endif

namespace hl = mf::http;
namespace api = mf::api;

using api::detail::STRequest;
using api::detail::SessionMaintainerRequest;

namespace {
struct SessionToken
{
    std::string session_token;
    std::string pkey;
    std::string time;
    int secret_key;
};

class AreStrictEquals : public boost::static_visitor<bool>
{
public:
    template <typename T, typename U>
    bool operator()( const T &, const U & ) const
    {
        return false; // cannot compare different types
    }

    template <typename T>
    bool operator()( const T & lhs, const T & rhs ) const
    {
        return lhs == rhs;
    }
};

template<typename T>
bool VariantsAreEqual(const T & lhs, const T & rhs)
{
    return boost::apply_visitor( AreStrictEquals(), lhs, rhs );
}

#ifdef OUTPUT_DEBUG // Debug code
std::ostream& operator<<(
        std::ostream& out,
        const api::SessionState & state
    )
{
    class DebugVisitor : public boost::static_visitor<std::string>
    {
    public:
        std::string operator()(api::session_state::Uninitialized) const
        {
            return "session_state::Uninitialized";
        }

        std::string operator()(api::session_state::Initialized) const
        {
            return "session_state::Initialized";
        }

        std::string operator()(api::session_state::CredentialsFailure) const
        {
            return "session_state::CredentialsFailure";
        }

        std::string operator()(api::session_state::Running) const
        {
            return "session_state::Running";
        }
    };

    out << boost::apply_visitor( DebugVisitor(), state );
    return out;
}
#endif

bool IsInitialized(const api::SessionState & state)
{
    return (boost::get<api::session_state::Initialized>(&state));
}
bool IsRunning(const api::SessionState & state)
{
    return (boost::get<api::session_state::Running>(&state));
}

}  // namespace

namespace mf {
namespace api {
namespace detail {

class SessionMaintainerLocker
{
public:
    SessionMaintainerLocker(
            boost::asio::io_service * work_ios,
            mf::utils::TimedActions<STRequest>::Callback callback
        ) :
        work_ios_(work_ios),
        session_state_(session_state::Uninitialized()),
        connection_state_(connection_state::Uninitialized()),
        session_state_change_count_(0),
        delayed_requests_(mf::utils::TimedActions<STRequest>::Create(
                work_ios, callback)),
        in_progress_session_token_requests_(0),
        time_out_requests_(mf::utils::TimedActions<STRequestWeak>::Create(
                work_ios, boost::bind(
                    &SessionMaintainerLocker::HandleTimedOutRequest, this, _1
                )))
    {
    }

    ~SessionMaintainerLocker()
    {
        delayed_requests_->Release();
        time_out_requests_->Release();
    }

    Credentials GetCredenials()
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        return credentials_;
    }
    void SetCredentials(
            const Credentials & credentials
        )
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        credentials_ = credentials;

        // All session tokens must become invalid.
        session_tokens_.clear();
        checked_out_tokens_.clear();

        // We have become initialized.
        ChangeSessionStateInternal(session_state::Initialized());
    }

    void AddWaitingRequest( STRequest request )
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);

        time_out_requests_->Add(
            std::chrono::seconds(request->TimeoutSeconds()),
            request );

        if (request->UsesSessionToken())
            waiting_st_requests_.push_back( request );
        else
            waiting_non_st_requests_.push_back( request );
    }

    void RemoveInProgressRequest( STRequest request )
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        in_progress_requests_.erase( request );
    }

    void MoveInProgressToWaiting( STRequest request )
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        in_progress_requests_.erase( request );

        if (request->UsesSessionToken())
            waiting_st_requests_.push_back( request );
        else
            waiting_non_st_requests_.push_back( request );
    }

    void DeleteCheckedOutToken( STRequest request )
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        auto it = checked_out_tokens_.find( request );
        if ( it != checked_out_tokens_.end() )
        {
            checked_out_tokens_.erase( it );
        }
    }

    void MoveInProgressToDelayed( STRequest request, int delay_sec )
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        in_progress_requests_.erase( request );
        delayed_requests_->Add(
            std::chrono::seconds(delay_sec),
            request );
    }

    boost::optional< STRequest > NextWaitingNonSessionTokenRequest()
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);

        if ( ! waiting_non_st_requests_.empty() )
        {
            STRequest request = waiting_non_st_requests_.front();
            waiting_non_st_requests_.pop_front();

            return request;
        }

        return boost::none;
    }

    boost::optional< std::pair<STRequest, SessionToken> >
        NextWaitingSessionTokenRequest()
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);

        if ( ! waiting_st_requests_.empty() && ! session_tokens_.empty() )
        {
            SessionToken st = session_tokens_.back();
            session_tokens_.pop_back();

            STRequest request = waiting_st_requests_.front();
            waiting_st_requests_.pop_front();

            return std::make_pair(std::move(request), std::move(st));
        }

        return boost::none;
    }

    void AddInProgressRequest( STRequest request )
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        in_progress_requests_.insert( request );
    }

    void AddInProgressRequest( STRequest request, SessionToken token )
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        in_progress_requests_.insert( request );
        checked_out_tokens_.insert(
            std::make_pair(std::move(request), std::move(token) ) );
    }

    bool PermitSessionTokenCheckout()
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        if (
            (  // Have at least one session token at all times
                in_progress_session_token_requests_ == 0
                &&
                session_tokens_.size()
                    + checked_out_tokens_.size() == 0
            )
            ||
            (  // And if already has one, get as many session tokens as needed
                (  /* Are there more requests than session tokens and session
                      token requests?  No check against checked out tokens, as
                      we allow maximum concurrent requests. */
                    waiting_st_requests_.size()
                    >
                    in_progress_session_token_requests_
                        + session_tokens_.size()
                )
                &&
                ( /* Are there less session token requests than maximum
                     concurrent session token requests allowed? */
                    in_progress_session_token_requests_
                    <
                    SessionMaintainer::max_in_progress_token_requests
                )
                &&
                ( /* Are there less session token requests and session tokens
                     than the maximum allowed session tokens? */
                    in_progress_session_token_requests_
                        + session_tokens_.size()
                        + checked_out_tokens_.size()
                    <
                    SessionMaintainer::max_tokens
                )
            )
        )
        {
            ++in_progress_session_token_requests_;
            return true;
        }
        return false;
    }

    void DecrementSessionTokenInProgressCount()
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        --in_progress_session_token_requests_;
    }

    bool AddSessionToken( SessionToken token, Credentials old_credentials )
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);

        if ( VariantsAreEqual( old_credentials, credentials_ ) )
        {
            session_tokens_.push_back( std::move(token) );
            return true;
        }
        else
        {
            return false;
        }
    }

    void ReuseToken(
            STRequest request,
            ResponseBase * response
        )
    {
        if (request->UsesSessionToken())
        {
            mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);

            auto it = checked_out_tokens_.find( request );
            if ( it != checked_out_tokens_.end() )
            {
                SessionToken st;
                std::swap( st, it->second );
                checked_out_tokens_.erase( it );

                // If a key was set to be renewed, we must modify the secret
                // key.
                if ( PropertyHasValue(
                        response->pt,
                        "response.new_key",
                        std::string("yes") ) )
                {
                    uint64_t current_key = st.secret_key;
                    uint64_t next_key = (current_key * 16807) % 2147483647;
                    st.secret_key = static_cast<int>(next_key);
                }

                session_tokens_.push_back( std::move(st) );
            }
        }
    }

    std::size_t TotalRequests()
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        return waiting_st_requests_.size()
            + waiting_non_st_requests_.size()
            + in_progress_requests_.size()
            + delayed_requests_->Size();
    }

    std::pair<api::SessionState, uint32_t> GetSessionState()
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        return std::make_pair(session_state_, session_state_change_count_);
    }

    void SetSessionState(api::SessionState state)
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        ChangeSessionStateInternal(state);
    }

    api::ConnectionState GetConnectionState()
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        return connection_state_;
    }

    void SetConnectionState(
            const api::ConnectionState & state
        )
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        ChangeConnectionStateInternal(state);
    }

    bool SetSessionStateSafe(
            api::SessionState state,
            uint32_t state_change_count
        )
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        if (session_state_change_count_ == state_change_count)
        {
            ChangeSessionStateInternal(state);
            return true;
        }
        return false;
    }

    void SetSessionStateChangeCallback(
            SessionMaintainer::SessionStateChangeCallback callback
        )
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        session_state_change_callback_ = callback;
    }

    void SetConnectionStateChangeCallback(
            SessionMaintainer::ConnectionStateChangeCallback callback
        )
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        connection_state_change_callback_ = callback;
    }

    void StopTimeouts()
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        delayed_requests_->Clear();
        time_out_requests_->Clear();
    }

private:
    boost::asio::io_service * work_ios_;

    mf::utils::mutex mutex_;

    // Set only via ChangeSessionStateInternal
    api::SessionState session_state_;

    // Set only via ChangeConnectionStateInternal
    api::ConnectionState connection_state_;

    uint32_t session_state_change_count_;
    SessionMaintainer::SessionStateChangeCallback
        session_state_change_callback_;

    SessionMaintainer::ConnectionStateChangeCallback
        connection_state_change_callback_;

    Credentials credentials_;

    std::deque< STRequest > waiting_st_requests_;
    std::deque< STRequest > waiting_non_st_requests_;
    std::set< STRequest > in_progress_requests_;

    std::map< STRequest, SessionToken > checked_out_tokens_;

    mf::utils::TimedActions<STRequest>::Pointer delayed_requests_;

    std::vector< SessionToken > session_tokens_;

    std::size_t in_progress_session_token_requests_;

    mf::utils::TimedActions<STRequestWeak>::Pointer time_out_requests_;

    void ChangeSessionStateInternal(
            api::SessionState state
        )
    {
        // No lock on this one since we should already be locked.
        if ( ! VariantsAreEqual( session_state_, state ) )
        {
            session_state_ = state;
            ++session_state_change_count_;
            if (session_state_change_callback_)
                work_ios_->post( boost::bind(
                        session_state_change_callback_,
                        session_state_) );
        }
    }

    void ChangeConnectionStateInternal(
            const api::ConnectionState & state
        )
    {
        // No lock on this one since we should already be locked.
        if ( ! VariantsAreEqual( connection_state_, state) )
        {
            connection_state_ = state;
            if (connection_state_change_callback_)
                work_ios_->post( boost::bind(
                        connection_state_change_callback_,
                        connection_state_) );
        }
    }

    void HandleTimedOutRequest(
            STRequestWeak weak_request
        )
    {
        STRequest request = weak_request.lock();

        if (request)
        {
            if (request->UsesSessionToken())
            {
                mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
                std::deque< STRequest >::iterator it = std::find(
                    waiting_st_requests_.begin(), waiting_st_requests_.end(),
                    request);
                if (it != waiting_st_requests_.end())
                {
                    waiting_st_requests_.erase(it);
                    request->Fail(
                        make_error_code(
                            api::errc::SessionTokenUnavailableTimeout ),
                        "No session token was available before the timeout was"
                        " reached."
                    );
                }
            }
            else
            {
                mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
                std::deque< STRequest >::iterator it = std::find(
                    waiting_non_st_requests_.begin(),
                    waiting_non_st_requests_.end(),
                    request);
                if (it != waiting_non_st_requests_.end())
                {
                    waiting_non_st_requests_.erase(it);
                    request->Fail(
                        make_error_code(
                            api::errc::ConnectionUnavailableTimeout ),
                        "connection unavailable before the timeout was reached."
                    );
                }
            }
        }
    }
};
}  // namespace detail

}  // namespace api
}  // namespace mf

api::SessionMaintainer::SessionMaintainer(
        mf::http::HttpConfig::ConstPointer http_config
    ) : SessionMaintainer(http_config, "www.mediafire.com")
{
}

api::SessionMaintainer::SessionMaintainer(
        mf::http::HttpConfig::ConstPointer http_config,
        std::string hostname
    ) :
    locker_(
        new detail::SessionMaintainerLocker(
            http_config->GetWorkIoService(),
            boost::bind(
                &api::SessionMaintainer::AddWaitingRequest,
                this, _1))),
    http_config_(http_config),
    session_token_failure_timer_(*http_config_->GetWorkIoService()),
    requester_(http_config_, hostname),
    timeout_seconds_(60)
{
    assert(http_config);
}

api::SessionMaintainer::~SessionMaintainer()
{
    auto work_ios = http_config_->GetWorkIoService();

    // There may be work still on the io service.  It must be stopped before we
    // can be destroyed, else there may be memory corruption.
    assert(work_ios->stopped());
    work_ios->stop();
}

void api::SessionMaintainer::SetLoginCredentials(
        const Credentials & credentials
    )
{
    // In case there are failures while this is occurring...
    SetSessionState(session_state::Uninitialized());

    locker_->SetCredentials(credentials);

    // Credentials updated.
    AttemptRequests();
}

void api::SessionMaintainer::HandleCompletion(
        STRequest request,
        ResponseBase * response
    )
{
    locker_->RemoveInProgressRequest(request);

    // Reuse token later if possible
    if ( IsInvalidSessionTokenError(response->error_code) )
        locker_->DeleteCheckedOutToken(request);
    else
        locker_->ReuseToken(request, response);
}

void api::SessionMaintainer::HandleRetryRequest(
        STRequest request,
        ResponseBase * response
    )
{
    // Put the request back on the queue. Give it a new token when available.
    {
        if ( IsInvalidSessionTokenError(response->error_code) )
        {   // Token invalid- delete it
            locker_->MoveInProgressToWaiting( request );
            locker_->DeleteCheckedOutToken( request );
        }
        else
        {   // Reuse token later
            locker_->MoveInProgressToDelayed(request, 1);
            locker_->ReuseToken(request, response);
        }
    }
    AttemptRequests();
}

void api::SessionMaintainer::HandleCompletionNotification(
        STRequest /* request */,
        ResponseBase * response
    )
{
    UpdateStateFromErrorCode(response->error_code);
}

void api::SessionMaintainer::UpdateStateFromErrorCode(
        const std::error_code & ec
    )
{
    using hl::http_error;

    if (ec)
    {
        if ( ec.category() == hl::http_category()
            && (    ec == http_error::UnableToConnect
                ||  ec == http_error::UnableToConnectToProxy
                ||  ec == http_error::UnableToResolve
                ||  ec == http_error::SslHandshakeFailure
                ||  ec == http_error::WriteFailure
                ||  ec == http_error::ReadFailure
                ||  ec == http_error::ProxyProtocolFailure
                ||  ec == http_error::IoTimeout
            ))
        {
            connection_state::Unconnected new_state = {ec};

            SetConnectionState(new_state);
        }
        else
        {
            SetConnectionState(connection_state::Connected());
        }
    }
    else
    {
        SetConnectionState(connection_state::Connected());
    }
}

void api::SessionMaintainer::AttemptRequests()
{
    // Handle requests that do not use session tokens.
    boost::optional< STRequest > opt_request;
    while ( (opt_request = locker_->NextWaitingNonSessionTokenRequest()) )
    {
        STRequest & request = *opt_request;

        request->Init(&requester_);
        locker_->AddInProgressRequest(std::move(request));
    }

    // Handle requests that have session tokens.
    boost::optional< std::pair<STRequest, SessionToken> > request_pair;
    while ( IsRunning(GetSessionState())
        && (request_pair = locker_->NextWaitingSessionTokenRequest()) )
    {
        SessionToken & st = request_pair->second;
        STRequest & request = request_pair->first;

        request->SetSessionToken(
                st.session_token,
                st.time,
                st.secret_key
            );
        request->Init(&requester_);

        locker_->AddInProgressRequest(std::move(request), std::move(st));
    }

    // Request more session tokens for those that need it.
    const Credentials credentials( locker_->GetCredenials() );

    while ( locker_->PermitSessionTokenCheckout() )
    {
        RequestSessionToken(credentials);
    }
}

void api::SessionMaintainer::RequestSessionToken(
        const Credentials & credentials
    )
{
#   ifdef OUTPUT_DEBUG // Debug code
    std::cout << "SessionMaintainer: Requesting new session token."
        << std::endl;
#   endif

    hl::HttpRequest::Pointer http_request =
        requester_.Call(
            api::user::get_session_token::Request(
                credentials
            ),
            boost::bind(
                &api::SessionMaintainer::HandleSessionTokenResponse,
                this,
                _1,
                credentials
            ),
            RequestStarted::No
        );

    // The timeout for these can be shorter.
    http_request->SetTimeout(15);

    http_request->Start();
}

void api::SessionMaintainer::HandleSessionTokenResponse(
        const api::user::get_session_token::Response & response,
        const Credentials & old_credentials
    )
{
    locker_->DecrementSessionTokenInProgressCount();

    api::SessionState session_state;
    uint32_t session_state_change_count;

    std::tie(session_state, session_state_change_count) =
        locker_->GetSessionState();

    UpdateStateFromErrorCode(response.error_code);

    if ( response.error_code )
    {
#       ifdef OUTPUT_DEBUG // Debug code
        std::cout << "SessionMaintainer: Session token request failed: "
            << response.error_code.message() << std::endl;
#       endif

        // Fail and set timeout before retry.
        session_token_failure_timer_.expires_from_now(
            boost::posix_time::milliseconds(
                session_token_failure_wait_timeout_ms) );

        session_token_failure_timer_.async_wait(
            boost::bind(
                &api::SessionMaintainer::HandleSessionTokenFailureTimeout,
                this,
                boost::asio::placeholders::error
            )
        );

        // If username or password is incorrect, stop.
        if (response.error_code == api::errc::CredentialsInvalid)
        {
            if (IsInitialized(session_state) || IsRunning(session_state))
            {
                // State is valid.

                const Credentials credentials = locker_->GetCredenials();
                // Equality check ensures this isn't an old request
                if ( VariantsAreEqual( old_credentials, credentials ) )
                {
                    session_state::CredentialsFailure new_state;
                    new_state.session_token_response = response;
                    new_state.error_code = response.error_code;

                    // If the pkey was sent, then the password may have been
                    // changed.  Let the class user figure this out if so.
                    if ( ! response.pkey.empty() )
                        new_state.pkey = response.pkey;

                    // This async call used the same credentials we currently
                    // have!  This means the username and password is bad.
                    locker_->SetSessionStateSafe(
                        new_state,
                        session_state_change_count
                        );
                }
            }
        }
    }
    else
    {
        // Session token is good.

#       ifdef OUTPUT_DEBUG // Debug code
        std::cout << "SessionMaintainer: Session token request success."
            << std::endl;
#       endif

        SessionToken st;

        st.session_token = response.session_token;
        st.pkey = response.pkey;
        st.secret_key = response.secret_key;
        st.time = response.time;

        if ( locker_->AddSessionToken(std::move(st), old_credentials) )
        {
            // Fix state if we are just starting.
            if (IsInitialized(session_state))
            {
                session_state::Running new_state = {response};

                locker_->SetSessionStateSafe(
                    new_state,
                    session_state_change_count );
            }
        }

        AttemptRequests();
    }
}

void api::SessionMaintainer::HandleSessionTokenFailureTimeout(
        const boost::system::error_code & err
    )
{
    if (!err)
    {
        AttemptRequests();
    }
}

void api::SessionMaintainer::AddWaitingRequest(
        STRequest request
    )
{
    locker_->AddWaitingRequest(request);
    AttemptRequests();
}

api::SessionState
api::SessionMaintainer::GetSessionState()
{
    return locker_->GetSessionState().first;
}

void api::SessionMaintainer::SetSessionState(api::SessionState state)
{
    return locker_->SetSessionState(state);
}

api::ConnectionState
api::SessionMaintainer::GetConnectionState()
{
    return locker_->GetConnectionState();
}

void api::SessionMaintainer::SetConnectionState(
        const api::ConnectionState & state
    )
{
    return locker_->SetConnectionState(state);
}

void api::SessionMaintainer::SetSessionStateChangeCallback(
        SessionStateChangeCallback callback
    )
{
    return locker_->SetSessionStateChangeCallback(callback);
}

void api::SessionMaintainer::SetConnectionStateChangeCallback(
        ConnectionStateChangeCallback callback
    )
{
    return locker_->SetConnectionStateChangeCallback(callback);
}

void api::SessionMaintainer::StopTimeouts()
{
    locker_->StopTimeouts();
}
