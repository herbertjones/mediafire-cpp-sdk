/**
 * @file state_redirect.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <memory>

#include "boost/msm/front/state_machine_def.hpp"

#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/error.hpp"
#include "mediafire_sdk/http/http_config.hpp"
#include "mediafire_sdk/http/url.hpp"

namespace mf {
namespace http {
namespace detail {

class Redirect : public boost::msm::front::state<>
{
public:
    template <typename Event, typename FSM>
    void on_entry(Event const & evt, FSM & fsm)
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

        using mf::http::RedirectPolicy;

        auto policy = fsm.get_redirect_policy();

        if (policy == RedirectPolicy::Deny )
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
        }
        else if (policy == RedirectPolicy::DenyDowngrade && fsm.get_is_ssl()
            && redirect_url->scheme() == "http" )
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
        }
        else  // Allow
        {
            fsm.set_parsed_url(std::move(redirect_url));

            fsm.set_url(evt.redirect_url);

            // Close connection
            fsm.Disconnect();

            fsm.ProcessEvent(RedirectedEvent{});
        }
    }

    template <typename Event, typename FSM>
    void on_exit(Event const&, FSM &)
    {
    }

private:
};

}  // namespace detail
}  // namespace http
}  // namespace mf
