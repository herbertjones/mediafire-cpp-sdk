/**
 * @file state_proxy_connect.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <chrono>
#include <memory>
#include <sstream>

#include "boost/asio.hpp"
#include "boost/msm/front/state_machine_def.hpp"

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

class ProxyConnectData
{
public:
    ProxyConnectData() :
        cancelled(false)
    {}

    bool cancelled;
};
using ProxyConnectDataPointer = std::shared_ptr<ProxyConnectData>;


template <typename FSM>
void HandleProxyHeaderRead(
        FSM  & fsm,
        ProxyConnectDataPointer state_data,
        RacePreventer race_preventer,
        std::shared_ptr<asio::streambuf> headers_buf,
        const TimePoint start_time,
        const std::size_t bytes_transferred,
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
        if ( fsm.UsingProxy() )
        {
            ss << " Proxy: " << fsm.CurrentProxy().host;
            ss << ":" << fsm.CurrentProxy().port;
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
        ProxyConnectDataPointer state_data,
        RacePreventer race_preventer,
        const TimePoint start_time,
        const std::size_t bytes_transferred,
        const boost::system::error_code& err
    )
{
    using mf::http::http_error;

    // Stop processing if actions cancelled.
    if (state_data->cancelled == true)
        return;

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
                [fsmp, state_data, race_preventer, response, start_time](
                        const boost::system::error_code& ec,
                        std::size_t bytes_transferred
                    )
                {
                    HandleProxyHeaderRead(*fsmp, state_data, race_preventer,
                        response, start_time, bytes_transferred, ec);
                });
        }
        else
        {
            asio::async_read_until(
                *fsm.get_socket_wrapper()->Socket(),
                *response,
                "\r\n\r\n",
                [fsmp, state_data, race_preventer, response, start_time](
                        const boost::system::error_code& ec,
                        std::size_t bytes_transferred
                    )
                {
                    HandleProxyHeaderRead(*fsmp, state_data, race_preventer,
                        response, start_time, bytes_transferred, ec);
                });
        }
    }
    else
    {
        std::stringstream ss;
        ss << "Failure connecting to proxy.";
        if ( fsm.UsingProxy() )
        {
            ss << " Proxy: " << fsm.CurrentProxy().host;
            ss << ":" << fsm.CurrentProxy().port;
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

class ProxyConnect : public boost::msm::front::state<>
{
public:
    template <typename Event, typename FSM>
    void on_entry(Event const &, FSM & fsm)
    {
        auto state_data = std::make_shared<ProxyConnectData>();
        state_data_ = state_data;

        // We know we are using a proxy.

        const Url * url = fsm.get_parsed_url();

        // Tell it where we want to connect to.
        std::string connect_host;
        connect_host = url->host();
        connect_host += ':';

        if (url->port().empty())
        {
            if (url->scheme() == "https")
                connect_host += "443";
            else
                connect_host += "80";
        }
        else
        {
            connect_host += url->port();
        }

        // Create headers to send.
        std::ostringstream request_stream_local;
        request_stream_local << "CONNECT " << connect_host << " HTTP/1.1\r\n";
        request_stream_local << "User-Agent: HttpRequester\r\n";

        const bool is_ssl = fsm.get_is_ssl();

        // Send authorization if provided.
        std::string username, password;
        if ( fsm.UsingProxy() )
        {
            username = fsm.CurrentProxy().username;
            password = fsm.CurrentProxy().password;
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
                [fsmp, state_data, race_preventer, request, start_time](
                       const boost::system::error_code& ec,
                       std::size_t bytes_transferred
                    )
                {
                    // request passed to prevent it from getting freed
                    HandleProxyConnectWrite(*fsmp, state_data, race_preventer,
                        start_time, bytes_transferred, ec);
                });
        }
        else
        {
            boost::asio::async_write(
                *fsm.get_socket_wrapper()->Socket(),
                *request,
                [fsmp, state_data, race_preventer, request, start_time](
                       const boost::system::error_code& ec,
                       std::size_t bytes_transferred
                    )
                {
                    // request passed to prevent it from getting freed
                    HandleProxyConnectWrite(*fsmp, state_data, race_preventer,
                        start_time, bytes_transferred, ec);
                });
        }

    }

    template <typename Event, typename FSM>
    void on_exit(Event const&, FSM &)
    {
        state_data_->cancelled = true;
        state_data_.reset();
    }

private:
    ProxyConnectDataPointer state_data_;
};

}  // namespace detail
}  // namespace http
}  // namespace mf
