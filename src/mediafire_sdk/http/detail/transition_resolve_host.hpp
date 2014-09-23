/**
 * @file transition_resolve_host.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <memory>

#include "boost/asio.hpp"
#include "boost/lexical_cast.hpp"

#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/detail/race_preventer.hpp"
#include "mediafire_sdk/http/detail/timeouts.hpp"
#include "mediafire_sdk/http/error.hpp"
#include "mediafire_sdk/http/url.hpp"

namespace mf {
namespace http {
namespace detail {

class ResolveData
{
public:
    ResolveData(
            boost::asio::io_service * io_service
        ) :
        resolver(*io_service)
    {}

    asio::ip::tcp::resolver resolver;
};
using ResolveDataPointer = std::shared_ptr<ResolveData>;

template <typename FSM>
void HandleResolve(
        FSM & fsm,
        ResolveDataPointer /*state_data*/,
        RacePreventer race_preventer,
        const boost::system::error_code& err,
        asio::ip::tcp::resolver::iterator endpoint_iterator
    )
{
    using mf::http::http_error;

    // Skip if cancelled due to timeout.
    if ( ! race_preventer.IsFirst() ) return;

    fsm.ClearAsyncTimeout();  // Must stop timeout timer.

    if (!err)
    {
        fsm.ProcessEvent(ResolvedEvent{endpoint_iterator});
    }
    else
    {
        std::stringstream ss;
        ss << "Failure while resolving url(" << fsm.get_url() << ").";
        ss << " Error: " << err.message();
        fsm.ProcessEvent(ErrorEvent{
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
            Event const &,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        auto state_data = std::make_shared<ResolveData>(
            fsm.get_work_io_service());

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
        state_data->resolver.async_resolve(
            query,
            [fsmp, state_data, race_preventer](
                    const boost::system::error_code& ec,
                    boost::asio::ip::tcp::resolver::iterator iterator
                )
            {
                HandleResolve( *fsmp, state_data, race_preventer, ec, iterator);
            });
    }
};

}  // namespace detail
}  // namespace http
}  // namespace mf
