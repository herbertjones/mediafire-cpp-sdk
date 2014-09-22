/**
 * @file transition_config.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 *
 * Detailed message...
 */
#pragma once

#include "boost/bind.hpp"
#include "boost/asio.hpp"

#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/detail/race_preventer.hpp"
#include "mediafire_sdk/http/detail/timeouts.hpp"
#include "mediafire_sdk/http/error.hpp"

namespace mf {
namespace http {
namespace detail {

template <typename FSM>
void HandleConnect(
        std::shared_ptr<FSM> fsmp,
        RacePreventer race_preventer,
        const boost::system::error_code& err
    )
{
    using mf::http::http_error;

    // Skip if cancelled due to timeout.
    if ( ! race_preventer.IsFirst() ) return;

    fsmp->ClearAsyncTimeout();  // Must stop timeout timer.

    if (!err)
    {
        fsmp->ProcessEvent(ConnectedEvent{});
    }
    else
    {
        std::stringstream ss;
        ss << "Failure while connecting(" << fsmp->get_url() << ").";
        ss << " Error: " << err.message();
        fsmp->ProcessEvent(
            ErrorEvent{
            make_error_code(
                http_error::UnableToConnect ),
            ss.str()
            });
    }
}

struct ConnectAction
{
    template <typename FSM,typename SourceState,typename TargetState>
    void operator()(
            ResolvedEvent const & evt,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        // Must prime timeout for async actions.
        auto race_preventer = fsm.SetAsyncTimeout("connect", kConnectTimeout);

        auto socket_wrapper = fsm.get_socket_wrapper();
        auto fsmp = fsm.AsFrontShared();

        if ( fsm.get_is_ssl() )
        {
            assert(socket_wrapper->SslSocket());

            // Attempt a connection to each endpoint in the list until we
            // successfully establish a connection.
            boost::asio::async_connect(
                socket_wrapper->SslSocket()->lowest_layer(),
                evt.endpoint_iterator,
                [fsmp, race_preventer](
                        const boost::system::error_code& ec,
                        boost::asio::ip::tcp::resolver::iterator iterator
                    )
                {
                    HandleConnect(fsmp, race_preventer, ec);
                });
        }
        else
        {
            assert(socket_wrapper->Socket());

            // Attempt a connection to each endpoint in the list until we
            // successfully establish a connection.
            boost::asio::async_connect(
                *socket_wrapper->Socket(),
                evt.endpoint_iterator,
                [fsmp, race_preventer](
                        const boost::system::error_code& ec,
                        boost::asio::ip::tcp::resolver::iterator iterator
                    )
                {
                    HandleConnect(fsmp, race_preventer, ec);
                });
        }
    }
};

}  // namespace detail
}  // namespace http
}  // namespace mf
