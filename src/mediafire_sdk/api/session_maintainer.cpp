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

#include "mediafire_sdk/api/detail/session_maintainer_locker.hpp"
#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/utils/mutex.hpp"
#include "mediafire_sdk/utils/variant_comparison.hpp"

#if ! defined(NDEBUG)
// Counter to keep track of number of HttpRequests
boost::atomic<int> mf::api::detail::session_maintainer_request_count(0);
#endif

namespace hl = mf::http;
namespace api = mf::api;

using api::detail::STRequest;
using api::detail::SessionMaintainerRequest;

using SessionToken = api::detail::SessionMaintainerLocker::SessionToken;

namespace {
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
        if (response.error_code == api::result_code::CredentialsInvalid)
        {
            if (IsInitialized(session_state) || IsRunning(session_state))
            {
                // State is valid.

                const Credentials credentials = locker_->GetCredenials();
                // Equality check ensures this isn't an old request
                if ( mf::utils::AreVariantsEqual(old_credentials, credentials))
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
