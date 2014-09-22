/**
 * @file transition_config.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 *
 * Detailed message...
 */
#pragma once

#include "mediafire_sdk/http/error.hpp"
#include "mediafire_sdk/http/url.hpp"
#include "mediafire_sdk/http/detail/timeouts.hpp"
#include "mediafire_sdk/http/detail/race_preventer.hpp"

namespace mf {
namespace http {
namespace detail {

template <typename FSM>
void HandleResolve(
        std::shared_ptr<FSM> fsmp,
        RacePreventer race_preventer,
        const boost::system::error_code& err,
        asio::ip::tcp::resolver::iterator endpoint_iterator
    )
{
    using mf::http::http_error;

    // Skip if cancelled due to timeout.
    if ( ! race_preventer.IsFirst() ) return;

    fsmp->ClearAsyncTimeout();  // Must stop timeout timer.

    if (!err)
    {
        fsmp->ProcessEvent(ResolvedEvent{endpoint_iterator});
    }
    else
    {
        std::stringstream ss;
        ss << "Failure while resolving url(" << fsmp->get_url() << ").";
        ss << " Error: " << err.message();
        fsmp->ProcessEvent(ErrorEvent{
                make_error_code(
                    http_error::UnableToResolve ),
                ss.str()
            });
    }
}

struct ResolveHostAction
{
    template <typename Event, typename FSM,typename SourceState,typename TargetState>
    void operator()(
            Event const & evt,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        const Url * url = fsm.get_parsed_url();

        assert(url);

        std::string host = url->host();
        std::string port = url->port();
        if ( port.empty() )
            port = url->scheme();

        // Set proxy.
        if ( fsm.get_is_ssl() && fsm.get_https_proxy() )
        {
            host = fsm.get_https_proxy()->host;
            port = boost::lexical_cast<std::string>(fsm.get_https_proxy()->port);
        }
        else if ( ! fsm.get_is_ssl() && fsm.get_http_proxy() )
        {
            host = fsm.get_http_proxy()->host;
            port = boost::lexical_cast<std::string>(fsm.get_http_proxy()->port);
        }

        // Must prime timeout for async actions.
        auto race_preventer = fsm.SetAsyncTimeout("resolving", kResolvingTimeout);
        auto fsmp = fsm.AsFrontShared();

        asio::ip::tcp::resolver::query query(host, port);
        fsm.resolver_.async_resolve(
            query,
            [fsmp, race_preventer](
                    const boost::system::error_code& ec,
                    boost::asio::ip::tcp::resolver::iterator iterator
                )
            {
                HandleResolve(
                    fsmp,
                    race_preventer,
                    ec,
                    iterator);
            });
    }
};

}  // namespace detail
}  // namespace http
}  // namespace mf
