/**
 * @file transition_config.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 *
 * Detailed message...
 */
#pragma once

#include <memory>

#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/error.hpp"
#include "mediafire_sdk/http/http_config.hpp"
#include "mediafire_sdk/http/url.hpp"

namespace mf {
namespace http {
namespace detail {

struct RedirectAction
{
    template <typename Event, typename FSM,typename SourceState,typename TargetState>
    void operator()(
            Event const & evt,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        using mf::http::http_error;
        using mf::http::Url;

        std::unique_ptr<Url> redirect_url;
        try {
            redirect_url.reset(new Url(evt.redirect_url));
        }
        catch(mf::http::InvalidUrl & /*err*/)
        {
            std::stringstream ss;
            ss << "Redirect to " << evt.redirect_url
                << " invalid url.";
            ss << " Source URL: " << fsm.get_url();

            fsm.ProcessEvent(ErrorEvent{
                    make_error_code(
                        http_error::InvalidRedirectUrl ),
                    ss.str()
                });
            return;
        }

        switch ( fsm.get_redirect_policy() )
        {
            case mf::http::RedirectPolicy::DenyDowngrade:
                if ( fsm.get_is_ssl() && redirect_url->scheme() == "http" )
                {
                    std::stringstream ss;
                    ss << "Redirect to non-SSL " << evt.redirect_url
                        << " denied by current policy.";
                    ss << " Source URL: " << fsm.get_url();
                    fsm.ProcessEvent(ErrorEvent{
                            make_error_code(
                                http_error::RedirectPermissionDenied ),
                            ss.str()
                        });
                }  // No break
            case mf::http::RedirectPolicy::Allow:
                {
                    fsm.set_parsed_url(std::move(redirect_url));

                    fsm.set_url(evt.redirect_url);

                    // Close connection
                    fsm.Disconnect();

                    fsm.ProcessEvent(RedirectedEvent{});
                } break;
            case mf::http::RedirectPolicy::Deny:
                {
                    std::stringstream ss;
                    ss << "Redirect to " << evt.redirect_url
                        << " denied by current policy.";
                    ss << " Source URL: " << fsm.get_url();
                    fsm.ProcessEvent(ErrorEvent{
                            make_error_code(
                                http_error::RedirectPermissionDenied ),
                            ss.str()
                        });
                } break;
        }
    }
};

}  // namespace detail
}  // namespace http
}  // namespace mf
