/**
 * @file transition_ssl_handshake.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <sstream>

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"

#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/detail/race_preventer.hpp"
#include "mediafire_sdk/http/detail/timeouts.hpp"
#include "mediafire_sdk/http/error.hpp"
#include "mediafire_sdk/http/url.hpp"

namespace mf {
namespace http {
namespace detail {

template <typename FSM>
void HandleHandshake(
        FSM  & fsm,
        RacePreventer race_preventer,
        const boost::system::error_code& err
    )
{
    using mf::http::http_error;

    // Skip if cancelled due to timeout.
    if ( ! race_preventer.IsFirst() ) return;

    fsm.ClearAsyncTimeout();  // Must stop timeout timer.

    if (!err)
    {
        fsm.ProcessEvent(HandshakeEvent{});
    }
    else
    {
        std::stringstream ss;
        ss << "Failure in SSL handshake.";
        ss << " Error: " << err.message();
        fsm.ProcessEvent(
            ErrorEvent{
                make_error_code(
                    http_error::SslHandshakeFailure ),
                ss.str()
            });
    }
}

struct SSLHandshakeAction
{
    template <typename Event, typename FSM,typename SourceState,typename TargetState>
    void operator()(
            Event const & /*evt*/,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        auto ssl_socket = fsm.get_socket_wrapper()->SslSocket();
        const Url * url = fsm.get_parsed_url();

        // This disables the Nagle algorithm, as supposedly SSL already
        // does something similar and Nagle hurts SSL performance by a lot,
        // supposedly.
        // http://www.extrahop.com/post/blog/performance-metrics/to-nagle-or-not-to-nagle-that-is-the-question/
        ssl_socket->lowest_layer().set_option(asio::ip::tcp::no_delay(true));

        // Don't allow self signed certs.
        ssl_socket->set_verify_mode(fsm.get_ssl_verify_mode());

        // Properly walk the certificate chain.
        ssl_socket->set_verify_callback(
                boost::asio::ssl::rfc2818_verification(url->host())
                );

        // Must prime timeout for async actions.
        auto race_preventer = fsm.SetAsyncTimeout("ssl handshake",
            kSslHandshakeTimeout);
        auto fsmp = fsm.AsFrontShared();

        ssl_socket->async_handshake(
            boost::asio::ssl::stream_base::client,
            [fsmp, race_preventer](const boost::system::error_code& ec)
            {
                HandleHandshake(*fsmp, race_preventer, ec);
            });
    }
};

}  // namespace detail
}  // namespace http
}  // namespace mf
