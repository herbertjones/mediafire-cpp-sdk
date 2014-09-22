/**
 * @file transition_config.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 *
 * Detailed message...
 */
#pragma once

#include <chrono>
#include <memory>
#include <sstream>

#include "boost/asio.hpp"

#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/detail/race_preventer.hpp"
#include "mediafire_sdk/http/detail/timeouts.hpp"
#include "mediafire_sdk/http/detail/types.hpp"
#include "mediafire_sdk/http/error.hpp"
#include "mediafire_sdk/http/url.hpp"
#include "mediafire_sdk/utils/base64.hpp"

namespace mf {
namespace http {
namespace detail {

using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

template <typename FSM>
void HandleProxyHeaderRead(
        FSM  & fsm,
        RacePreventer race_preventer,
        std::shared_ptr<asio::streambuf> headers_buf,
        const TimePoint start_time,
        const std::size_t bytes_transferred,
        const boost::system::error_code& err
    )
{
    using mf::http::http_error;

    // Skip if cancelled due to timeout.
    if ( ! race_preventer.IsFirst() ) return;

    fsm.ClearAsyncTimeout();  // Must stop timeout timer.

    const bool is_ssl = fsm.get_is_ssl();

    if (fsm.get_bw_analyser())
    {
        fsm.get_bw_analyser()->RecordIncomingBytes( bytes_transferred,
            start_time, sclock::now() );
    }

    if (!err)
    {
        std::istream response_stream(headers_buf.get());

        std::string http_version;
        response_stream >> http_version;

        unsigned int status_code;
        response_stream >> status_code;

        std::string status_message;
        std::getline(response_stream, status_message);

        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
            std::stringstream ss;
            ss << "Protocol error while parsing proxy headers.";
            fsm.ProcessEvent(
                ErrorEvent{
                    make_error_code(
                        http_error::ProxyProtocolFailure ),
                    ss.str()
                });
            return;
        }

        if ( status_code == 200 )
        {
            fsm.ProcessEvent(ConnectedEvent{});
        }
        else
        {
            std::stringstream ss;
            ss << "Protocol error while parsing proxy headers.";
            ss << " HTTP Status: " << status_code;
            ss << " " << status_message;
            fsm.ProcessEvent(
                ErrorEvent{
                    make_error_code(
                        http_error::ProxyProtocolFailure ),
                    ss.str()
                });
        }
    }
    else
    {
        std::stringstream ss;
        ss << "Failure reading from proxy.";
        if ( is_ssl && fsm.get_https_proxy() )
        {
            ss << " Proxy: " << fsm.get_https_proxy()->host;
            ss << ":" << fsm.get_https_proxy()->port;
        }
        else if ( ! is_ssl && fsm.get_http_proxy() )
        {
            ss << " Proxy: " << fsm.get_http_proxy()->host;
            ss << ":" << fsm.get_http_proxy()->port;
        }

        ss << " Error: " << err.message();
        fsm.ProcessEvent(
            ErrorEvent{
                make_error_code(
                    http_error::ProxyProtocolFailure ),
                ss.str()
            });
    }
}

template <typename FSM>
void HandleProxyConnectWrite(
        FSM & fsm,
        RacePreventer race_preventer,
        const TimePoint start_time,
        const std::size_t bytes_transferred,
        const boost::system::error_code& err
    )
{
    using mf::http::http_error;

    const bool is_ssl = fsm.get_is_ssl();

    // Skip if cancelled due to timeout.
    if ( ! race_preventer.IsFirst() ) return;

    fsm.ClearAsyncTimeout();  // Must stop timeout timer.

    if (fsm.get_bw_analyser())
    {
        fsm.get_bw_analyser()->RecordOutgoingBytes( bytes_transferred,
            start_time, sclock::now() );
    }

    if (!err)
    {
        std::shared_ptr<asio::streambuf> response =
            std::make_shared<asio::streambuf>();

        // Must prime timeout for async actions.
        auto race_preventer = fsm.SetAsyncTimeout("proxy read response",
            kProxyReadTimeout);
        auto fsmp = fsm.AsFrontShared();
        auto start_time = sclock::now();

        // Must use direct socket instead of SSL stream here as we do
        // non-SSL communication with the proxy.
        if ( is_ssl )
        {
            asio::async_read_until(
                fsm.get_socket_wrapper()->SslSocket()->next_layer(),
                *response, "\r\n\r\n",
                [fsmp, race_preventer, response, start_time](
                        const boost::system::error_code& ec,
                        std::size_t bytes_transferred
                    )
                {
                    HandleProxyHeaderRead(*fsmp, race_preventer, response,
                        start_time, bytes_transferred, ec);
                });
        }
        else
        {
            asio::async_read_until(
                *fsm.get_socket_wrapper()->Socket(),
                *response,
                "\r\n\r\n",
                [fsmp, race_preventer, response, start_time](
                        const boost::system::error_code& ec,
                        std::size_t bytes_transferred
                    )
                {
                    HandleProxyHeaderRead(*fsmp, race_preventer, response,
                        start_time, bytes_transferred, ec);
                });
        }
    }
    else
    {
        std::stringstream ss;
        ss << "Failure connecting to proxy.";
        if (is_ssl && fsm.get_https_proxy())
        {
            ss << " Proxy: " << fsm.get_https_proxy()->host;
            ss << ":" << fsm.get_https_proxy()->port;
        }
        else if ( ! is_ssl && fsm.get_http_proxy())
        {
            ss << " Proxy: " << fsm.get_http_proxy()->host;
            ss << ":" << fsm.get_http_proxy()->port;
        }

        ss << " Error: " << err.message();
        fsm.ProcessEvent(
            ErrorEvent{
                make_error_code(
                    http_error::UnableToConnectToProxy ),
                ss.str()
            });
    }
}
struct ProxyConnectAction
{
    template <typename Event, typename FSM,typename SourceState,typename TargetState>
    void operator()(
            Event const & evt,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        // We know we are using a proxy.

        const Url * url = fsm.get_parsed_url();

        // Tell it where we want to connect to.
        std::string connect_host;
        connect_host = url->host();
        connect_host += ':';
        connect_host += url->port();

        // Create headers to send.
        std::ostringstream request_stream_local;
        request_stream_local << "CONNECT " << connect_host << " HTTP/1.1\r\n";
        request_stream_local << "User-Agent: HttpRequester\r\n";

        const bool is_ssl = fsm.get_is_ssl();

        // Send authorization if provided.
        std::string username, password;
        if (is_ssl && fsm.get_https_proxy())
        {
            username = fsm.get_https_proxy()->username;
            password = fsm.get_https_proxy()->password;
        }
        else if ( ! is_ssl && fsm.get_http_proxy())
        {
            username = fsm.get_http_proxy()->username;
            password = fsm.get_http_proxy()->password;
        }

        if ( ! username.empty())
        {
            std::string to_encode = username;
            to_encode += ':';
            to_encode += password;
            std::string encoded = mf::utils::Base64Encode(
                    to_encode.c_str(), to_encode.size() );

            request_stream_local << "Proxy-Authorization: Basic " << encoded
                << "\r\n";
        }

        request_stream_local << "\r\n";  // End header

        std::shared_ptr<boost::asio::streambuf> request(
            std::make_shared<boost::asio::streambuf>());

        std::ostream request_stream(request.get());
        request_stream << request_stream_local.str();

        // Must prime timeout for async actions.
        auto race_preventer = fsm.SetAsyncTimeout("proxy write request",
            kProxyWriteTimeout);
        auto fsmp = fsm.AsFrontShared();
        auto start_time = sclock::now();

        // Must use direct socket instead of SSL stream here as we do non-SSL
        // communication with the proxy.
        if ( is_ssl )
        {
            boost::asio::async_write(
                fsm.get_socket_wrapper()->SslSocket()->next_layer(),
                *request,
                [fsmp, race_preventer, request, start_time](
                       const boost::system::error_code& ec,
                       std::size_t bytes_transferred
                    )
                {
                    // request passed to prevent it from getting freed
                    HandleProxyConnectWrite(*fsmp, race_preventer, start_time,
                        bytes_transferred, ec);
                });
        }
        else
        {
            boost::asio::async_write(
                *fsm.get_socket_wrapper()->Socket(),
                *request,
                [fsmp, race_preventer, request, start_time](
                       const boost::system::error_code& ec,
                       std::size_t bytes_transferred
                    )
                {
                    // request passed to prevent it from getting freed
                    HandleProxyConnectWrite(*fsmp, race_preventer, start_time,
                        bytes_transferred, ec);
                });
        }
    }
};

}  // namespace detail
}  // namespace http
}  // namespace mf
