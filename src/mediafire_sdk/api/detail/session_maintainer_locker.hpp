/**
 * @file session_maintainer_locker.hpp
 * @author Herbert Jones
 * @brief Threadsafe impl for session maintainer
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include "mediafire_sdk/api/connection_state.hpp"
#include "mediafire_sdk/api/credentials.hpp"
#include "mediafire_sdk/api/detail/session_maintainer_request.hpp"
#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/session_state.hpp"
#include "mediafire_sdk/utils/timed_events.hpp"

namespace mf {
namespace api {
namespace detail {

class SessionMaintainerLocker
{
public:
    struct SessionToken
    {
        std::string session_token;
        std::string pkey;
        std::string time;
        int secret_key;
    };

    using TimedEvents = mf::utils::TimedEvent<STRequest>;
    using WeakTimedEvents = mf::utils::TimedEvent<STRequestWeak>;

    SessionMaintainerLocker(
            boost::asio::io_service * work_ios,
            TimedEvents::EventProcessor event_processor
        );

    ~SessionMaintainerLocker();

    WeakTimedEvents::EventProcessor CreateTimeoutProcessor();

    Credentials GetCredenials();

    void SetCredentials(
            const Credentials & credentials
        );

    void AddWaitingRequest( STRequest request );

    void RemoveInProgressRequest( STRequest request );

    void MoveInProgressToWaiting( STRequest request );

    void DeleteCheckedOutToken( STRequest request );

    void MoveInProgressToDelayed( STRequest request, int delay_sec );

    boost::optional< STRequest > NextWaitingNonSessionTokenRequest();

    boost::optional< std::pair<STRequest, SessionToken> >
        NextWaitingSessionTokenRequest();

    void AddInProgressRequest( STRequest request );

    void AddInProgressRequest( STRequest request, SessionToken token );

    bool PermitSessionTokenCheckout();

    void DecrementSessionTokenInProgressCount();

    bool AddSessionToken( SessionToken token, Credentials old_credentials );

    void ReuseToken(
            STRequest request,
            ResponseBase * response
        );

    std::size_t TotalRequests();

    std::pair<api::SessionState, uint32_t> GetSessionState();

    void SetSessionState(api::SessionState state);

    api::ConnectionState GetConnectionState();

    void SetConnectionState(
            const api::ConnectionState & state
        );

    bool SetSessionStateSafe(
            api::SessionState state,
            uint32_t state_change_count
        );

    void SetSessionStateChangeCallback(
            SessionMaintainer::SessionStateChangeCallback callback
        );

    void SetConnectionStateChangeCallback(
            SessionMaintainer::ConnectionStateChangeCallback callback
        );

    void StopTimeouts();

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

    TimedEvents::Pointer delayed_requests_;

    std::vector< SessionToken > session_tokens_;

    std::size_t in_progress_session_token_requests_;

    WeakTimedEvents::Pointer time_out_requests_;

    void ChangeSessionStateInternal(
            api::SessionState state
        );

    void ChangeConnectionStateInternal(
            const api::ConnectionState & state
        );

    void HandleTimedOutRequest(
            STRequestWeak weak_request
        );
};

}  // namespace detail
}  // namespace api
}  // namespace mf
