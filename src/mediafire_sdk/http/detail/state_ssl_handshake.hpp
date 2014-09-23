/**
 * @file state_ssl_handshake.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <sstream>

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/msm/front/state_machine_def.hpp"

#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/detail/race_preventer.hpp"
#include "mediafire_sdk/http/detail/timeouts.hpp"
#include "mediafire_sdk/http/error.hpp"
#include "mediafire_sdk/http/url.hpp"

namespace mf {
namespace http {
namespace detail {

class SslConnectData
{
public:
    SslConnectData() :
        cancelled(false)
    {}

    bool cancelled;
};
using SslConnectDataPointer = std::shared_ptr<SslConnectData>;

template <typename FSM>
void HandleHandshake(
        FSM  & fsm,
        SslConnectDataPointer state_data,
        RacePreventer race_preventer,
        const boost::system::error_code& err
    )
{
    using mf::http::http_error;

    // Stop processing if actions cancelled.
    if (state_data->cancelled == true)
        return;

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

class SSLHandshake : public boost::msm::front::state<>
{
public:
    template <typename Event, typename FSM>
    void on_entry(Event const &, FSM & fsm)
    {
        auto state_data = std::make_shared<SslConnectData>();
        state_data_ = state_data;

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
            [fsmp, state_data, race_preventer](
                    const boost::system::error_code& ec
                )
            {
                HandleHandshake(*fsmp, state_data, race_preventer, ec);
            });

    }

    template <typename Event, typename FSM>
    void on_exit(Event const&, FSM &)
    {
        state_data_->cancelled = true;
        state_data_.reset();
    }

private:
    SslConnectDataPointer state_data_;
};

}  // namespace detail
}  // namespace http
}  // namespace mf
