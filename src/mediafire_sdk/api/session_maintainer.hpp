/**
 * @file session_maintainer.hpp
 * @author Herbert Jones
 * @brief Class to maintain a list of session tokens.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <functional>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "mediafire_sdk/api/session_maintainer_request.hpp"
#include "mediafire_sdk/api/user/get_session_token.hpp"
#include "mediafire_sdk/api/credentials.hpp"
#include "mediafire_sdk/api/connection_state.hpp"
#include "mediafire_sdk/api/session_state.hpp"

#include "mediafire_sdk/utils/timed_actions.hpp"

namespace mf {
namespace api {

namespace detail {
class SessionMaintainerLocker;
}  // namespace detail

/**
 * @class SessionMaintainer
 * @brief Maintains a list of session tokens, which are required to make most
 * API requests and keeps status of obtaining those tokens.
 */
class SessionMaintainer
{
public:
    /** Class static constants */
    enum {
        max_in_progress_token_requests = 4,
        max_tokens = 10,
        session_token_failure_wait_timeout_ms = 2500,
    };

    /** Signature for session state change callbacks */
    typedef std::function<void(api::SessionState)> SessionStateChangeCallback;

    /** Signature for connection state change callbacks */
    typedef std::function<void(api::ConnectionState)>
        ConnectionStateChangeCallback;

    /**
     * @brief Create the session token maintainer.
     *
     * @param[in] http_config HttpConfig object
     * @param[in] bwa Bandwidth analyser
     */
    SessionMaintainer(
            mf::http::HttpConfig::ConstPointer http_config
        );

    /**
     * @brief Create the session token maintainer.
     *
     * @param[in] http_config HttpConfig object
     * @param[in] hostname Hostname to use for API requests.
     */
    SessionMaintainer(
            mf::http::HttpConfig::ConstPointer http_config,
            std::string hostname
        );

    ~SessionMaintainer();

    /**
     * @brief Set the callback for session state changes.
     *
     * @param[in] callback Function to call on change.
     */
    void SetSessionStateChangeCallback(
            SessionStateChangeCallback callback
        );

    /**
     * @brief Set the callback for connection state changes.
     *
     * @param[in] callback Function to call on change.
     */
    void SetConnectionStateChangeCallback(
            ConnectionStateChangeCallback callback
        );

    /**
     * @brief Set the credentials.
     *
     * The credentials must be set before any calls that require a session token
     * can be performed.
     *
     * @param[in] credentials The credentials for the account.
     */
    void SetLoginCredentials(
            const Credentials & credentials
        );

    /**
     * @brief Make an API call.
     *
     * @param[in] api_container The API object which holds all the data with
     * which to make the call.
     * @param[in] callback Function called when the API request completes.
     * @param[in] ios IO service where callback will be performed.
     *
     * @return API request object
     */
    template<typename ApiFunctor, typename IoService>
    Request Call(
            ApiFunctor api_container,
            typename ApiFunctor::CallbackType callback,
            IoService * callback_io_service
        )
    {
        typedef detail::SessionMaintainerRequest<ApiFunctor, IoService> StImpl;

        typedef std::function<void(detail::STRequest, ResponseBase*)>
            CompletionCallback;

        CompletionCallback on_complete = boost::bind(
            &api::SessionMaintainer::HandleCompletion, this, _1, _2);
        CompletionCallback on_retry_request = boost::bind(
            &api::SessionMaintainer::HandleRetryRequest, this, _1, _2);
        CompletionCallback on_completion = boost::bind(
            &api::SessionMaintainer::HandleCompletionNotification,
            this, _1, _2);

        typename StImpl::Pointer request = StImpl::Create(
            &api_container,
            callback,
            callback_io_service,
            timeout_seconds_,
            on_complete,
            on_retry_request,
            on_completion );

        // SessionMaintainer maintain the life of request.
        AddWaitingRequest(request);
        AttemptRequests();

        return request;
    }

    /**
     * @brief Make an API call.
     *
     * The callback will be performed in the work thread passed in during
     * creation of the SessionMaintainer.
     *
     * @param[in] api_container The API object which holds all the data with
     * which to make the call.
     * @param[in] callback Function called when the API request completes.
     *
     * @return API request object
     */
    template<typename ApiFunctor>
    Request Call(
            ApiFunctor api_container,
            typename ApiFunctor::CallbackType callback
        )
    {
        return Call( api_container, callback,
            http_config_->GetDefaultCallbackIoService());
    }

    /**
     * @brief Set the normal number of timeout seconds.
     *
     * This sets the HTTP connect and retry timeout duration. Certain
     * error codes are retried automatically. This timeout prevents the request
     * from trying infinitely for such errors.
     *
     * @param[in] timeout_seconds Seconds till request timeout.
     */
    void SetTimeoutSeconds(uint32_t timeout_seconds)
    {
        timeout_seconds_ = timeout_seconds;
    }

    /**
     * @brief Stop all io_service timeouts so io_service::run can return as soon
     * as there is no longer any work to perform.
     *
     * This is mostly useful for unit tests.
     */
    void StopTimeouts();

    mf::http::HttpConfig::ConstPointer HttpConfig() const {return http_config_;}

private:
    std::unique_ptr<detail::SessionMaintainerLocker> locker_;

    mf::http::HttpConfig::ConstPointer http_config_;
    boost::asio::deadline_timer session_token_failure_timer_;

    Requester requester_;

    uint32_t timeout_seconds_;

    api::SessionState GetSessionState();
    void SetSessionState(api::SessionState);

    api::ConnectionState GetConnectionState();
    void SetConnectionState(const api::ConnectionState &);

    void HandleCompletion(
            detail::STRequest request,
            ResponseBase * response
        );
    void HandleRetryRequest(
            detail::STRequest request,
            ResponseBase * response
        );
    void HandleCompletionNotification(
            detail::STRequest request,
            ResponseBase * response);

    void UpdateStateFromErrorCode(const std::error_code &);

    void AttemptRequests();
    void RequestSessionToken(
            const Credentials & credentials
        );

    void HandleSessionTokenResponse(
            const api::user::get_session_token::Response & response,
            const Credentials & credentials
        );

    void HandleDelayedRequestTimeout( detail::STRequest );

    void HandleSessionTokenFailureTimeout(
            const boost::system::error_code & err
        );

    void AddWaitingRequest( detail::STRequest request );
};

}  // namespace api
}  // namespace mf
