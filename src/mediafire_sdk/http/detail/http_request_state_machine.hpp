/**
 * @file http_request_state_machine.hpp
 * @author Herbert Jones
 * @brief Http Request state machine
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <iostream>

// back-end
#include "boost/msm/back/state_machine.hpp"
// front-end
#include "boost/msm/front/state_machine_def.hpp"

#include "boost/algorithm/string/case_conv.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/asio/steady_timer.hpp"
#include "boost/bind.hpp"
#include "boost/iostreams/copy.hpp"
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/restrict.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/variant/apply_visitor.hpp"

#if ! defined(NDEBUG)
#   include "boost/atomic.hpp"
#endif

#include "mediafire_sdk/http/detail/default_http_headers.hpp"
#include "mediafire_sdk/http/detail/http_request_events.hpp"

#include "mediafire_sdk/http/post_data_pipe_interface.hpp"
#include "mediafire_sdk/http/error.hpp"
#include "mediafire_sdk/http/shared_buffer.hpp"

#include "mediafire_sdk/utils/base64.hpp"
#include "mediafire_sdk/utils/string.hpp"

// #define OUTPUT_DEBUG

namespace mf {
namespace http {
namespace detail {


// Forward declarations
class HttpRequestMachine_;

/** Back-end to HttpRequest state machine.  Use this class. */
using HttpRequestMachine =
    boost::msm::back::state_machine<detail::HttpRequestMachine_>;


using sclock = std::chrono::steady_clock;
using Duration = std::chrono::milliseconds;
using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

struct HttpRequestMachineConfig
{
    HttpConfig::ConstPointer http_config;
    const std::string & url;
    std::shared_ptr<RequestResponseInterface> callbk;
    boost::asio::io_service * callback_io_service;
};

template<typename T>
Duration AsDuration(const T & t)
{
    return std::chrono::duration_cast<Duration>(t);
}

class RacePreventer;

namespace hl = mf::http;
namespace msm = boost::msm;
namespace mpl = boost::mpl;
namespace asio = boost::asio;
namespace mfr = boost::msm::front;

const int kResolvingTimeout = 30;
const int kSslHandshakeTimeout = 30;
const int kConnectTimeout = 30;
const int kProxyWriteTimeout = 30;
const int kProxyReadTimeout = 30;

/**
 * @class SocketWrapper
 * @brief Wrapper to prevent deleting sockets that have pending operations.
 *
 * Cancelling and deleting a socket while the socket has ongoing operations does
 * not cancel the ongoing operations immediately.  Asio may still be writing to
 * the SSL "socket" with new data, and deleting that stream while it is in use
 * does cause memory access violations.  This class prevents a socket from being
 * deleted until we are absolutely sure the socket is no longer being used, and
 * informs the callback function for the asio operation that the operation is no
 * longer valid.
 */
class SocketWrapper
{
public:
    explicit SocketWrapper(
            asio::ssl::stream<asio::ip::tcp::socket> * socket
        )
    {
        ssl_socket_.reset(socket);
    }

    explicit SocketWrapper(
            asio::ip::tcp::socket * socket
        )
    {
        socket_.reset(socket);
    }

    /**
     * @brief DTOR
     *
     * We can destroy the socket now that all operations have ceased on it.
     */
    ~SocketWrapper()
    {
        if (ssl_socket_)
        {
            ssl_socket_.reset();
        }
        else if (socket_)
        {
            socket_.reset();
        }
    }

    /**
     * @brief Access the SSL socket.
     *
     * @return Pointer to the SSL socket
     */
    asio::ssl::stream<asio::ip::tcp::socket> * SslSocket()
    {
        return ssl_socket_.get();
    }

    /**
     * @brief Access the non-SSL socket.
     *
     * @return Pointer to the non-SSL socket
     */
    asio::ip::tcp::socket * Socket()
    {
        return socket_.get();
    }

    /**
     * @brief Cancel asynchronous operations on socket.
     */
    void Cancel()
    {
        if ( ssl_socket_ )
        {
            boost::system::error_code ec;
            ssl_socket_->lowest_layer().cancel(ec);
        }
        else if (socket_)
        {
            boost::system::error_code ec;
            socket_->cancel(ec);
        }
    }

// private: -- clang on OSX 10.8 doesn't like friend class
    friend class RacePreventer;

    std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> ssl_socket_;
    std::shared_ptr<asio::ip::tcp::socket> socket_;
};

/**
 * @class RacePreventer
 * @brief Prevent race conditions due to deadline timer asio issues.
 */
class RacePreventer
{
public:
    RacePreventer(SocketWrapper * wrapper) :
        value_(std::make_shared<bool>(false)),
        ssl_socket_(wrapper->ssl_socket_),
        socket_(wrapper->socket_)
    {}

    bool IsFirst()
    {
        auto orignal_value = *value_;
        *value_ = true;
        return ! orignal_value;
    }

private:
    std::shared_ptr<bool> value_;

    // SocketWrapper will release the socket on restart when a timeout occurs
    // and the async operation is cancelled.  Due to the asynchronous nature,
    // the socket read or write operation might still be on the io_service
    // function queue.  The socket needs to exist until the cancelled operation
    // calls its callback handler with operation_aborted.  We hold on to them
    // here, and when the cancelled callback completes, the race preventer
    // returns false on IsFirst, and then the socket will be destroyed when the
    // last RacePreventer is destroyed.
    std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> ssl_socket_;
    std::shared_ptr<asio::ip::tcp::socket> socket_;
};



static const uint64_t kMaxUnknownReadLength = 1024 * 8;

enum TransferEncoding
{
    TE_None          =  0,
    TE_Unknown       = (1<<0),

    TE_ContentLength = (1<<1),
    TE_Chunked       = (1<<2),
    TE_Gzip          = (1<<3),
};

enum ContentEncoding
{
    CE_None    =  0,
    CE_Unknown = (1<<0),

    CE_Gzip    = (1<<1),
};

float MultiplierFromPercent(float percent)
{
    // If desired bandwidth usage is 95% of total bandwidth:
    // 95 * n = 5
    // n = 5 / 95
    // n = (100 - 95) / 95
    if (percent > 100.0f)
    {
        assert(!"Bandwidth usage percent greater than 100");
        percent = 100.0f;
    }
    else if (percent < 1.0f)
    {
        assert(!"Bandwidth usage percent less than 1");
        percent = 1.0f;
    }
    return ( 100.0f - percent ) / percent;
}

int ParseTransferEncoding(
        const std::map<std::string, std::string> & headers
    )
{
    // See http://www.iana.org/assignments/http-parameters/http-parameters.xhtml

    int ret = static_cast<int>(TE_None);

    auto it = headers.find("content-length");
    if ( it != headers.end() )
        ret |= static_cast<int>(TE_ContentLength);

    it = headers.find("transfer-encoding");
    if ( it != headers.end() )
    {
        std::vector<std::string> tokens;
        boost::split(tokens, it->second, boost::is_any_of(","));
        std::for_each(std::begin(tokens), std::end(tokens),
                [](std::string & v){
                    boost::trim(v);
                } );

        for (const std::string & token : tokens)
        {
            if ( token == "chunked" )
                ret |= static_cast<int>(TE_Chunked);
            else if ( token == "gzip" )
                ret |= static_cast<int>(TE_Gzip);
            else
                ret |= static_cast<int>(TE_Unknown);
        }
    }

    return ret;
}

int ParseContentEncoding(
        const std::map<std::string, std::string> & headers
    )
{
    int ret = static_cast<int>(CE_None);

    auto it = headers.find("content-encoding");

    if ( it != headers.end() )
    {
        std::vector<std::string> tokens;
        boost::split(tokens, it->second, boost::is_any_of(","));
        std::for_each(std::begin(tokens), std::end(tokens),
                [](std::string & v){
                    boost::trim(v);
                } );

        for (const std::string & token : tokens)
        {
            if ( token == "gzip" )
                ret |= static_cast<int>(CE_Gzip);
            else
                ret |= static_cast<int>(CE_Unknown);
        }
    }

    return ret;
}

class HttpRequestBuffer : public mf::http::BufferInterface
{
public:
    HttpRequestBuffer(std::unique_ptr<uint8_t[]> buffer, uint64_t size) :
        buffer_(std::move(buffer)),
        size_(size)
    {}
    virtual ~HttpRequestBuffer() {}

    virtual uint64_t Size() const
    {
        return size_;
    }

    /**
     * @brief Get buffer
     *
     * @return pointer that can be read with data size contained in Size().
     */
    virtual const uint8_t * Data() const
    {
        return buffer_.get();
    }
private:
    std::unique_ptr<uint8_t[]> buffer_;
    uint64_t size_;
};

class VectorBuffer : public mf::http::BufferInterface
{
public:
    VectorBuffer() {}
    virtual ~VectorBuffer() {}

    virtual uint64_t Size() const
    {
        return buffer.size();
    }

    /**
     * @brief Get buffer
     *
     * @return pointer that can be read with data size contained in Size().
     */
    virtual const uint8_t * Data() const
    {
        return buffer.data();
    }

    std::vector<uint8_t> buffer;
private:
};

asio::ssl::verify_mode CertAllowToVerifyMode(hl::SelfSigned ss)
{
    switch(ss)
    {
        case hl::SelfSigned::Denied:
            return asio::ssl::verify_peer;
        case hl::SelfSigned::Permitted:
        default:
            return asio::ssl::verify_none;
    }
}

// front-end: define the FSM structure
class HttpRequestMachine_ :
    public std::enable_shared_from_this<HttpRequestMachine_>,
    public msm::front::state_machine_def<HttpRequestMachine_>
{
public:
    typedef std::shared_ptr<HttpRequestMachine_> Pointer;

    // Warning: Not possible to add more than 5 arguments here:
    HttpRequestMachine_( const HttpRequestMachineConfig & config ) :
        http_config_(config.http_config),
        work_io_service_(config.http_config->GetWorkIoService()),
        event_strand_(*work_io_service_),
        bw_analyser_(http_config_->GetBandwidthAnalyser()),
        request_creation_time_(sclock::now()),
        transmission_delay_timer_(*work_io_service_),
        transmission_delay_timer_enabled_(false),
        timer_(*work_io_service_),
        timeout_seconds_(60),
        timeout_id_(0),
        callback_(config.callbk),
        callback_io_service_(config.callback_io_service),
        resolver_(*work_io_service_),
        redirect_policy_(http_config_->GetRedirectPolicy()),
        ssl_ctx_(http_config_->GetSslContext()),
        ssl_verify_mode_(CertAllowToVerifyMode(
                http_config_->SelfSignedCertificatesAllowed())),
        request_method_("GET"),
        url_(config.url),
        original_url_(config.url),
        post_interface_size_(0),
        post_interface_read_bytes_(0),
        headers_(http_config_->GetDefaultHeaders()),
        http_proxy_(http_config_->GetHttpProxy()),
        https_proxy_(http_config_->GetHttpsProxy()),
        filter_buf_consumed_(0),
        delay_multiplier_(MultiplierFromPercent(
                http_config_->GetBandwidthUsagePercent()))
    {
#if ! defined(NDEBUG)
        const int object_count = request_count_.fetch_add(1,
            boost::memory_order_relaxed);
#       ifdef OUTPUT_DEBUG // Debug code
        std::cout << "++HttpRequests: " << (object_count+1) << ' ' << url_
            << std::endl;
#       endif
        assert( object_count < 100 );
#endif
    }

    virtual ~HttpRequestMachine_()
    {
#if ! defined(NDEBUG)
        const int object_count = request_count_.fetch_sub(1,
            boost::memory_order_release);
#       ifdef OUTPUT_DEBUG // Debug code
        std::cout << "--HttpRequests: " << (object_count-1) << ' ' << url_
            << std::endl;
#       endif
        assert(object_count > 0);
#endif
    }

    template<typename Event>
    void ProcessEvent(Event evt)
    {
        Pointer self(shared_from_this());

        event_strand_.dispatch(
            [this, self, evt]()
            {
                msm::back::state_machine<HttpRequestMachine_> &fsm =
                    static_cast<msm::back::state_machine<HttpRequestMachine_>&>(
                        *this);
                fsm.process_event( evt );
            });
    }

    // The list of FSM states

    // Notmal states -----
    struct Unstarted : public msm::front::state<>
    {
#if 0
        // It is possible to add on_entry and on_exit actions.

        template <typename Event, typename FSM>
        void on_entry(Event const& , FSM&)
        {
            std::cout << "entering: Unstarted" << std::endl;
        }

        template <typename Event, typename FSM>
        void on_exit(Event const&, FSM& )
        {
            std::cout << "leaving: Unstarted" << std::endl;
        }
#endif
    };
    struct Initializing : public msm::front::state<> {};
    struct Resolve : public msm::front::state<> {};
    struct SSLHandshake : public msm::front::state<> {};
    struct Connect : public msm::front::state<> {};
    struct SendHeader : public msm::front::state<> {};
    struct ProxyConnect : public msm::front::state<> {};
    struct SendPost : public msm::front::state<> {};
    struct ReadHeaders : public msm::front::state<> {};
    struct ParseHeaders : public msm::front::state<> {};
    struct ReadContent : public msm::front::state<> {};
    struct Redirect : public msm::front::state<> {};
    struct Error : public msm::front::state<> {};

    // Terminate states -----
    struct FinalError : public msm::front::terminate_state<>
    {
        template <typename Event, typename FSM>
        void on_entry(Event const& evt , FSM& machine)  // NOLINT
        {
            assert( machine.event_strand_.running_in_this_thread() );

            // Close connection
            machine.Disconnect();

            if ( machine.transmission_delay_timer_enabled_ )
            {
                machine.transmission_delay_timer_enabled_ = false;
                machine.transmission_delay_timer_.cancel();
            }

            std::shared_ptr<hl::RequestResponseInterface> iface(
                    machine.callback_);
            machine.callback_io_service_->dispatch(
                    [evt, iface](){
                        iface->RequestResponseErrorEvent(
                            evt.code, evt.description );
                    }
                );
            }
    };
    struct Complete : public msm::front::terminate_state<>
    {
        template <typename Event, typename FSM>
        void on_entry(Event const& , FSM& machine)  // NOLINT
        {
            assert( machine.event_strand_.running_in_this_thread() );

            // Close connection
            machine.Disconnect();

            std::shared_ptr<hl::RequestResponseInterface> iface(
                    machine.callback_);

            machine.callback_io_service_->dispatch(
                    [iface](){
                        iface->RequestResponseCompleteEvent();
                    }
                );
        }
    };

    // the initial state of the HttpRequestMachine SM. Must be defined
    typedef Unstarted initial_state;

    class ConfigEventHandler
        : public boost::static_visitor<>
    {
    public:
        explicit ConfigEventHandler( HttpRequestMachine_ * m) :
            machine_(m)
        {}

        void operator()(const ConfigEvent::ConfigRedirectPolicy & cfg) const
        {
            machine_->redirect_policy_ = cfg.redirect_policy;
        }

        void operator()(const ConfigEvent::ConfigRequestMethod & cfg) const
        {
            machine_->request_method_ = cfg.method;
        }

        void operator()(const ConfigEvent::ConfigHeader & cfg) const
        {
            machine_->SetHeader(cfg.name, cfg.value);
        }

        void operator()(const ConfigEvent::ConfigPostDataPipe & cfg) const
        {
            machine_->post_data_.reset();
            machine_->post_interface_ = cfg.pdi;
            machine_->SetHeader("Content-Length",
                mf::utils::to_string(cfg.pdi->PostDataSize()));

            machine_->request_method_ = "POST";
        }

        void operator()(const ConfigEvent::ConfigPostData & cfg) const
        {
            machine_->post_data_ = cfg.raw_data;
            machine_->post_interface_.reset();
            machine_->SetHeader("Content-Length",
                mf::utils::to_string(cfg.raw_data->Size()));

            machine_->request_method_ = "POST";
        }

        void operator()( const ConfigEvent::ConfigTimeout & cfg) const
        {
            machine_->timeout_seconds_ = cfg.timeout_seconds;
        }

    private:
        HttpRequestMachine_ * machine_;
    };
    friend class ConfigEventHandler;

    // transition actions
    void ConfigEventAction(ConfigEvent const& evt)
    {
        boost::apply_visitor( ConfigEventHandler(this), evt.variant );
    }
    template<typename Event>
    void InitializeAction(Event const&)
    {
        InitializeAction_();
    }
    void InitializeAction_()
    {
        // We may be here due to new connection or due to a redirect, so
        // ensure we are initialized as if new.

        // We should not be connected at this point.
        Disconnect();

        // There should be nothing in the buffers.
        read_buffer_.consume( read_buffer_.size() );
        gzipped_buffer_.consume( gzipped_buffer_.size() );

        filter_buf_consumed_ = 0;

        // Parse URL if redirect didn't pass one in.
        try {
            if ( ! parsed_url_ )
                parsed_url_.reset(new hl::Url(url_));
        }
        catch(hl::InvalidUrl & err)
        {
            std::stringstream ss;
            ss << "Bad url(url:";
            ss << url_;
            ss << " reason: " << err.what() << ")";

            ProcessEvent(ErrorEvent{
                    make_error_code( hl::http_error::InvalidUrl ),
                    ss.str()
                });
            return;
        }

        if ( parsed_url_->scheme() == "http" )
        {
            is_ssl_ = false;

            // Create non-SSL socket and wrapper.
            socket_wrapper_ = std::make_shared<SocketWrapper>(
                new asio::ip::tcp::socket(*work_io_service_) );

            ProcessEvent(InitializedEvent());
        }
        else if ( parsed_url_->scheme() == "https" )
        {
            is_ssl_ = true;

            // Create the SSL socket and wrapper
            socket_wrapper_ = std::make_shared<SocketWrapper>(
                new asio::ssl::stream<asio::ip::tcp::socket>(
                    *work_io_service_, *ssl_ctx_ ) );

            ProcessEvent(InitializedEvent());
        }
        else
        {
            std::stringstream ss;
            ss << "Unsupported scheme. Url: " << url_;
            ProcessEvent(ErrorEvent{
                    make_error_code( hl::http_error::InvalidUrl ),
                    ss.str()
                });
        }
    }
    template<typename Event>
    void ResolveHostAction(Event const& /* evt */)
    {
        std::string host = parsed_url_->host();
        std::string port = parsed_url_->port();
        if ( port.empty() )
            port = parsed_url_->scheme();

        // Set proxy.
        if ( is_ssl_ && https_proxy_ )
        {
            host = https_proxy_->host;
            port = boost::lexical_cast<std::string>(https_proxy_->port);
        }
        else if ( ! is_ssl_ && http_proxy_ )
        {
            host = http_proxy_->host;
            port = boost::lexical_cast<std::string>(http_proxy_->port);
        }

        // Must prime timeout for async actions.
        auto race_preventer = SetAsyncTimeout("resolving", kResolvingTimeout);

        asio::ip::tcp::resolver::query query(host, port);
        resolver_.async_resolve(
            query,
            boost::bind(
                &HttpRequestMachine_::HandleResolve, shared_from_this(),
                race_preventer,
                asio::placeholders::error,
                asio::placeholders::iterator
            ));
    }
    void VerifyErrorAction(ErrorEvent const& evt)
    {
        assert( event_strand_.running_in_this_thread() );

        // Close connection
        Disconnect();

        auto timeout = request_creation_time_ +
            std::chrono::seconds(timeout_seconds_);

        if (evt.code == hl::http_error::IoTimeout && sclock::now() < timeout)
        {
            // Restart everything
            ProcessEvent(RestartEvent{});
        }
        else
        {
            // Pass error on
            ProcessEvent(evt);
        }
    }
    void SSLHandshakeAction(ConnectedEvent const& /* evt */)
    {
        // This disables the Nagle algorithm, as supposedly SSL already
        // does something similar and Nagle hurts SSL performance by a lot,
        // supposedly.
        // http://www.extrahop.com/post/blog/performance-metrics/to-nagle-or-not-to-nagle-that-is-the-question/
        socket_wrapper_->SslSocket()->lowest_layer().set_option(
                asio::ip::tcp::no_delay(true)
            );

        // Don't allow self signed certs.
        socket_wrapper_->SslSocket()->set_verify_mode(ssl_verify_mode_);

        // Properly walk the certificate chain.
        socket_wrapper_->SslSocket()->set_verify_callback(
                asio::ssl::rfc2818_verification(parsed_url_->host())
                );

        // Must prime timeout for async actions.
        auto race_preventer = SetAsyncTimeout("ssl handshake",
            kSslHandshakeTimeout);

        socket_wrapper_->SslSocket()->async_handshake(
            boost::asio::ssl::stream_base::client,
            boost::bind(
                &HttpRequestMachine_::HandleHandshake,
                shared_from_this(),
                race_preventer,
                boost::asio::placeholders::error));
    }
    void ConnectAction(ResolvedEvent const& evt)
    {
        // Must prime timeout for async actions.
        auto race_preventer = SetAsyncTimeout("connect", kConnectTimeout);

        if ( is_ssl_ )
        {
            assert(socket_wrapper_->SslSocket());

            // Attempt a connection to each endpoint in the list until we
            // successfully establish a connection.
            asio::async_connect(
                socket_wrapper_->SslSocket()->lowest_layer(),
                evt.endpoint_iterator,
                boost::bind(
                    &HttpRequestMachine_::HandleConnect,
                    shared_from_this(),
                    race_preventer,
                    asio::placeholders::error));
        }
        else
        {
            assert(socket_wrapper_->Socket());

            // Attempt a connection to each endpoint in the list until we
            // successfully establish a connection.
            asio::async_connect(
                *socket_wrapper_->Socket(),
                evt.endpoint_iterator,
                boost::bind(
                    &HttpRequestMachine_::HandleConnect,
                    shared_from_this(),
                    race_preventer,
                    asio::placeholders::error));
        }
    }
    template<typename Event>
    void ProxyConnectAction(Event const&)
    {
        // We know we are using a proxy.

        // Tell it where we want to connect to.
        std::string connect_host;
        connect_host = parsed_url_->host();
        connect_host += ':';
        connect_host += parsed_url_->port();

        // Create headers to send.
        std::ostringstream request_stream_local;
        request_stream_local << "CONNECT " << connect_host << " HTTP/1.1\r\n";
        request_stream_local << "User-Agent: HttpRequester\r\n";

        // Send authorization if provided.
        std::string username, password;
        if (is_ssl_ && https_proxy_)
        {
            username = https_proxy_->username;
            password = https_proxy_->password;
        }
        else if ( ! is_ssl_ && http_proxy_)
        {
            username = http_proxy_->username;
            password = http_proxy_->password;
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

        std::shared_ptr<asio::streambuf> request(
            std::make_shared<asio::streambuf>());

        std::ostream request_stream(request.get());
        request_stream << request_stream_local.str();

        // Must prime timeout for async actions.
        auto race_preventer = SetAsyncTimeout("proxy write request",
            kProxyWriteTimeout);

        if ( is_ssl_ )
        {
            asio::async_write(
                socket_wrapper_->SslSocket()->next_layer(),
                *request,
                boost::bind(
                    &HttpRequestMachine_::HandleProxyConnectWrite,
                    shared_from_this(),
                    race_preventer,
                    request,
                    sclock::now(),
                    asio::placeholders::bytes_transferred,
                    asio::placeholders::error));
        }
        else
        {
            asio::async_write(
                *socket_wrapper_->Socket(),
                *request,
                boost::bind(
                    &HttpRequestMachine_::HandleProxyConnectWrite,
                    shared_from_this(),
                    race_preventer,
                    request,
                    sclock::now(),
                    asio::placeholders::bytes_transferred,
                    asio::placeholders::error));
        }
    }
    template<typename Event>
    void SendHeaderAction(Event const&)
    {
        // When a proxy is used, the full path must be sent.
        std::string path = parsed_url_->full_path();
        if (     ( is_ssl_ && https_proxy_ )
            || ( ! is_ssl_ && http_proxy_ ) )
        {
            path = parsed_url_->url();
        }

        std::ostringstream request_stream_local;
        request_stream_local << request_method_ << " "
            << path << " HTTP/1.1\r\n";
        request_stream_local << "Host: " << parsed_url_->host() << "\r\n";

        if ( ! is_ssl_ && http_proxy_ && ! http_proxy_->username.empty() )
        {
            std::string to_encode = http_proxy_->username;
            to_encode += ':';
            to_encode += http_proxy_->password;
            std::string encoded = mf::utils::Base64Encode(
                    to_encode.c_str(), to_encode.size() );

            request_stream_local << "Proxy-Authorization: Basic " << encoded
                << "\r\n";
        }

        for ( const auto & pair : headers_ )
        {
            if ( is_ssl_ && boost::iequals(pair.first, "accept-encoding" ) )
            {
                // Compression over SSL is not allowed as it is a vulnerability.
                // See BREACH.
                continue;
            }

            request_stream_local
                << pair.first << ": "  // name
                << pair.second  // value
                << "\r\n";  // HTTP header newline
        }

        request_stream_local << "\r\n";  // End header with blank line.

        std::shared_ptr<asio::streambuf> request(
            std::make_shared<asio::streambuf>());

        std::ostream request_stream(request.get());
        request_stream << request_stream_local.str();

        // Must prime timeout for async actions.
        auto race_preventer = SetAsyncTimeout("write request header",
            timeout_seconds_);

        if ( is_ssl_ )
        {
            asio::async_write(
                *socket_wrapper_->SslSocket(),
                *request,
                boost::bind(
                    &HttpRequestMachine_::HandleHeaderWrite,
                    shared_from_this(),
                    race_preventer,
                    request,
                    sclock::now(),
                    asio::placeholders::bytes_transferred,
                    asio::placeholders::error));
        }
        else
        {
            asio::async_write(
                *socket_wrapper_->Socket(),
                *request,
                boost::bind(
                    &HttpRequestMachine_::HandleHeaderWrite,
                    shared_from_this(),
                    race_preventer,
                    request,
                    sclock::now(),
                    asio::placeholders::bytes_transferred,
                    asio::placeholders::error));
        }
    }
    void SendPostAction(HeadersWrittenEvent const&)
    {
        if ( post_data_ )
        {
            // Must prime timeout for async actions.
            auto race_preventer = SetAsyncTimeout("write request post",
                timeout_seconds_);

            if ( is_ssl_ )
            {
                asio::async_write(
                    *socket_wrapper_->SslSocket(),
                    asio::buffer(post_data_->Data(), post_data_->Size()),
                    boost::bind(
                        &HttpRequestMachine_::HandlePostWrite,
                        shared_from_this(),
                        race_preventer,
                        post_data_,
                        sclock::now(),
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error));
            }
            else
            {
                asio::async_write(
                    *socket_wrapper_->Socket(),
                    asio::buffer(post_data_->Data(), post_data_->Size()),
                    boost::bind(
                        &HttpRequestMachine_::HandlePostWrite,
                        shared_from_this(),
                        race_preventer,
                        post_data_,
                        sclock::now(),
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error));
            }
        }
        else
        {
            post_interface_size_ = post_interface_->PostDataSize();

            const auto now = std::chrono::steady_clock::now();
            PostViaInterface( now, now );
        }
    }
    void PostViaInterface(
            TimePoint last_write_start,
            TimePoint last_write_end
        )
    {
        hl::SharedBuffer::Pointer post_data;

        try {
            post_data = post_interface_->RetreivePostDataChunk();
        }
        catch( const std::exception & err )
        {
            std::stringstream ss;
            ss << "Failure to retrieve POST data from interface.";
            ss << " Error: " << err.what();
            ProcessEvent(ErrorEvent{
                    make_error_code(
                        hl::http_error::VariablePostInterfaceFailure ),
                    ss.str()
                });
        }

        if ( ! post_data || post_data->Size() == 0 )
        {
            // Allow interface to free.
            post_interface_.reset();

            if ( post_interface_read_bytes_!= post_interface_size_ )
            {
                std::stringstream ss;
                ss << "POST data unexpected size.";
                ss << " Expected: " << post_interface_size_;
                ss << " Received: " << post_interface_read_bytes_;
                ProcessEvent(ErrorEvent{
                        make_error_code(
                            hl::http_error::VariablePostInterfaceFailure ),
                        ss.str()
                    });
            }
            else
            {
                ProcessEvent(PostSent{});
            }
        }
        else
        {
            post_interface_read_bytes_ += post_data->Size();

            // Delay behavior:
            SetTransactionDelayTimer( last_write_start, last_write_end );
            std::function<void()> strand_wrapped = std::bind(
                    &HttpRequestMachine_::PostViaInterfaceDelayCallback,
                    shared_from_this(),
                    post_data
                );
            transmission_delay_timer_.async_wait(
                event_strand_.wrap(
                    boost::bind(
                        &HttpRequestMachine_::SetTransactionDelayTimerWrapper,
                        shared_from_this(),
                        strand_wrapped,
                        asio::placeholders::error
                    )));
        }
    }
    void PostViaInterfaceDelayCallback(
            hl::SharedBuffer::Pointer post_data
        )
    {
        // Ensure a cancellation doesn't mess up the state due to async
        // timer.
        if ( transmission_delay_timer_enabled_ )
        {
            transmission_delay_timer_enabled_ = false;

            // Must prime timeout for async actions.
            auto race_preventer = SetAsyncTimeout("write request post",
                timeout_seconds_);

            if ( is_ssl_ )
            {
                asio::async_write(
                    *socket_wrapper_->SslSocket(),
                    asio::buffer(post_data->Data(), post_data->Size()),
                    boost::bind(
                        &HttpRequestMachine_::HandlePostWrite,
                        shared_from_this(),
                        race_preventer,
                        post_data,
                        sclock::now(),
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error));
            }
            else
            {
                asio::async_write(
                    *socket_wrapper_->Socket(),
                    asio::buffer(post_data->Data(), post_data->Size()),
                    boost::bind(
                        &HttpRequestMachine_::HandlePostWrite,
                        shared_from_this(),
                        race_preventer,
                        post_data,
                        sclock::now(),
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error));
            }
        }
    }
    template<typename Event>
    void ReadHeaderAction(Event const&)
    {
        // Must prime timeout for async actions.
        auto race_preventer = SetAsyncTimeout("read response header",
            timeout_seconds_);

        if ( is_ssl_ )
        {
            asio::async_read_until(*socket_wrapper_->SslSocket(), read_buffer_,
                "\r\n\r\n",
                    boost::bind(
                        &HttpRequestMachine_::HandleHeaderRead,
                        shared_from_this(),
                        race_preventer,
                        sclock::now(),
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error));
        }
        else
        {
            asio::async_read_until(*socket_wrapper_->Socket(), read_buffer_,
                "\r\n\r\n",
                    boost::bind(
                        &HttpRequestMachine_::HandleHeaderRead,
                        shared_from_this(),
                        race_preventer,
                        sclock::now(),
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error));
        }
    }
    void ParseHeadersAction(HeadersReadEvent const& evt)
    {
        if (evt.status_code == 301 || evt.status_code == 302)
        {
            auto it = evt.headers.find("location");
            if ( it == evt.headers.end() )
            {
                std::stringstream ss;
                ss << "Bad " << evt.status_code << " redirect.";
                ss << " Source URL: " << url_;
                ss << " Missing \"Location\" header";
                ProcessEvent(ErrorEvent{
                        make_error_code(
                            hl::http_error::InvalidRedirectUrl ),
                        ss.str()
                    });
            }
            else
            {
                RedirectEvent redirect(evt);
                redirect.redirect_url = it->second;
                ProcessEvent(redirect);
            }
        }
        else
        {
            read_headers_ = evt;

            ::mf::http::Headers headers;

            headers.raw_headers = evt.raw_headers;
            headers.status_code = evt.status_code;
            headers.status_message = evt.status_message;
            headers.headers = evt.headers;

            std::shared_ptr<hl::RequestResponseInterface>
                iface(callback_);
            callback_io_service_->dispatch(
                    [iface, headers](){
                        iface->ResponseHeaderReceived(
                            headers );
                    }
                );

            ProcessEvent(HeadersParsedEvent{});
        }
    }
    void ReadContentAction(HeadersParsedEvent const&)
    {
        // Encodings!
        int te = ParseTransferEncoding(read_headers_.headers);
        int ce = ParseContentEncoding(read_headers_.headers);

        // gzip can be in transfer-encoding or content-encoding...
        if ( te & TE_Gzip )
            ce |= static_cast<int>(CE_Gzip);

        if ( te & TE_Unknown )
        {
            std::stringstream ss;
            ss << "Unsupported transfer-encoding.";
            auto it = read_headers_.headers.find("transfer-encoding");
            if ( it != read_headers_.headers.end() )
                ss << " Transfer-Encoding: " << it->second;
            ProcessEvent(ErrorEvent{
                    make_error_code(
                        hl::http_error::UnsupportedEncoding ),
                    ss.str()
                });
            return;
        }

        if ( ce & CE_Unknown )
        {
            std::stringstream ss;
            ss << "Unsupported content-encoding.";
            auto it = read_headers_.headers.find("content-encoding");
            if ( it != read_headers_.headers.end() )
                ss << " Content-Encoding: " << it->second;
            ProcessEvent(ErrorEvent{
                    make_error_code(
                        hl::http_error::UnsupportedEncoding ),
                    ss.str()
                });
            return;
        }

        if ( ce & CE_Gzip )
        {
            filter_buf_.push(boost::iostreams::gzip_decompressor());
        }

        if ( te & TE_Chunked && te & TE_ContentLength )
        {
            std::stringstream ss;
            ss << "Unable to handle chunked encoding and content length.";
            ss << " Violates RFC 2616, Section 4.4";
            auto it = read_headers_.headers.find("content-encoding");
            if ( it != read_headers_.headers.end() )
                ss << " Content-Encoding: " << it->second;
            it = read_headers_.headers.find("content-length");
            if ( it != read_headers_.headers.end() )
                ss << " Content-Length: " << it->second;
            ProcessEvent(ErrorEvent{
                    make_error_code(
                        hl::http_error::UnsupportedEncoding ),
                    ss.str()
                });
            return;
        }

        if ( te & TE_Chunked )
        {
            // Must prime timeout for async actions.
            auto race_preventer = SetAsyncTimeout("read response content 1",
                timeout_seconds_);

            if ( is_ssl_ )
            {
                asio::async_read_until(
                    *socket_wrapper_->SslSocket(), read_buffer_, "\r\n",
                    boost::bind(
                        &HttpRequestMachine_::HandleContentChunkSizeRead,
                        shared_from_this(),
                        race_preventer,
                        0,
                        te, ce,
                        sclock::now(),
                        Duration::zero(),
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error
                        )
                    );
            }
            else
            {
                asio::async_read_until(
                    *socket_wrapper_->Socket(), read_buffer_, "\r\n",
                    boost::bind(
                        &HttpRequestMachine_::HandleContentChunkSizeRead,
                        shared_from_this(),
                        race_preventer,
                        0,
                        te, ce,
                        sclock::now(),
                        Duration::zero(),
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error
                        )
                    );
            }
        }
        else
        {
            uint64_t max_read_size = kMaxUnknownReadLength;
            if ( te & TE_ContentLength )
            {
                max_read_size = std::min(
                    read_headers_.content_length,
                    kMaxUnknownReadLength );
            }

            // Must prime timeout for async actions.
            auto race_preventer = SetAsyncTimeout("read response content 2",
                timeout_seconds_);

            if ( is_ssl_ )
            {
                /* hjones note: asio::transfer_exactly might not be needed here
                 * as SSL may take care of the proper read size for us */
                asio::async_read(*socket_wrapper_->SslSocket(), read_buffer_,
                    asio::transfer_exactly(max_read_size),
                    boost::bind(
                        &HttpRequestMachine_::HandleContentRead,
                        shared_from_this(),
                        race_preventer,
                        0,
                        te, ce,
                        sclock::now(),
                        read_buffer_.size(),
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error
                    )
                );
            }
            else
            {
                asio::async_read(*socket_wrapper_->Socket(), read_buffer_,
                    asio::transfer_exactly(max_read_size),
                    boost::bind(
                        &HttpRequestMachine_::HandleContentRead,
                        shared_from_this(),
                        race_preventer,
                        0,
                        te, ce,
                        sclock::now(),
                        read_buffer_.size(),
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error
                        )
                    );
            }
        }
    }
    void RedirectAction(RedirectEvent const& evt)
    {
        std::unique_ptr<hl::Url> redirect_url;
        try {
            redirect_url.reset(new hl::Url(evt.redirect_url));
        }
        catch(hl::InvalidUrl & /*err*/)
        {
            std::stringstream ss;
            ss << "Redirect to " << evt.redirect_url
                << " invalid url.";
            ss << " Source URL: " << url_;

            ProcessEvent(ErrorEvent{
                    make_error_code(
                        hl::http_error::InvalidRedirectUrl ),
                    ss.str()
                });
            return;
        }

        switch ( redirect_policy_ )
        {
            case hl::RedirectPolicy::DenyDowngrade:
                if ( is_ssl_ && redirect_url->scheme() == "http" )
                {
                    std::stringstream ss;
                    ss << "Redirect to non-SSL " << evt.redirect_url
                        << " denied by current policy.";
                    ss << " Source URL: " << url_;
                    ProcessEvent(ErrorEvent{
                            make_error_code(
                                hl::http_error::RedirectPermissionDenied ),
                            ss.str()
                        });
                }  // No break
            case hl::RedirectPolicy::Allow:
                {
                    parsed_url_.swap(redirect_url);
                    url_ = evt.redirect_url;

                    // Close connection
                    Disconnect();

                    ProcessEvent(RedirectedEvent{});
                } break;
            case hl::RedirectPolicy::Deny:
                {
                    std::stringstream ss;
                    ss << "Redirect to " << evt.redirect_url
                        << " denied by current policy.";
                    ss << " Source URL: " << url_;
                    ProcessEvent(ErrorEvent{
                            make_error_code(
                                hl::http_error::RedirectPermissionDenied ),
                            ss.str()
                        });
                } break;
        }
    }

    // Guards

    template<typename Event>
    bool HasPost(Event const &)
    {
        return static_cast<bool>(post_data_)
            || static_cast<bool>(post_interface_);
    }
    template<typename Event>
    bool HasNoPost(Event const & evt)
    {
        return ! HasPost(evt);
    }

    template<typename Event>
    bool IsSslAndProxy(Event const &)
    {
        return is_ssl_ && https_proxy_;
    }
    template<typename Event>
    bool IsSslAndNotProxy(Event const &)
    {
        return is_ssl_ && ! https_proxy_;
    }
    template<typename Event>
    bool IsSsl(Event const &)
    {
        return is_ssl_;
    }
    template<typename Event>
    bool IsNotSsl(Event const &)
    {
        return ! is_ssl_;
    }

    // Asio Handlers

    void HandleAsyncTimeout(
            RacePreventer race_preventer,
            uint32_t timeout_id,
            const boost::system::error_code& err
        )
    {
        // Skip if other function beat us to it.
        if ( ! race_preventer.IsFirst() ) return;

        // If timeout id doesn't match, then both timeout handler and async
        // handler ended up on the stack at the same time.
        if (!err && timeout_id_ == timeout_id )
        {
            std::ostringstream ss;
            ss << "I/O timeout: " << timeout_reason_;
            ProcessEvent(ErrorEvent{
                    make_error_code( hl::http_error::IoTimeout ),
                    ss.str()
                });
        }
    }
    void HandleResolve(
            RacePreventer race_preventer,
            const boost::system::error_code& err,
            asio::ip::tcp::resolver::iterator endpoint_iterator
        )
    {
        // Skip if cancelled due to timeout.
        if ( ! race_preventer.IsFirst() ) return;

        ClearAsyncTimeout();  // Must stop timeout timer.

        if (!err)
        {
            ProcessEvent(ResolvedEvent{endpoint_iterator});
        }
        else
        {
            std::stringstream ss;
            ss << "Failure while resolving url(" << url_ << ").";
            ss << " Error: " << err.message();
            ProcessEvent(ErrorEvent{
                    make_error_code(
                        hl::http_error::UnableToResolve ),
                    ss.str()
                });
        }
    }
    void HandleHandshake(
            RacePreventer race_preventer,
            const boost::system::error_code& err
        )
    {
        // Skip if cancelled due to timeout.
        if ( ! race_preventer.IsFirst() ) return;

        ClearAsyncTimeout();  // Must stop timeout timer.

        if (!err)
        {
            ProcessEvent(HandshakeEvent{});
        }
        else
        {
            std::stringstream ss;
            ss << "Failure in SSL handshake.";
            ss << " Error: " << err.message();
            ProcessEvent(
                ErrorEvent{
                    make_error_code(
                        hl::http_error::SslHandshakeFailure ),
                    ss.str()
                });
        }
    }
    void HandleConnect(
            RacePreventer race_preventer,
            const boost::system::error_code& err
        )
    {
        // Skip if cancelled due to timeout.
        if ( ! race_preventer.IsFirst() ) return;

        ClearAsyncTimeout();  // Must stop timeout timer.

        if (!err)
        {
            ProcessEvent(ConnectedEvent{});
        }
        else
        {
            std::stringstream ss;
            ss << "Failure while connecting(" << url_ << ").";
            ss << " Error: " << err.message();
            ProcessEvent(
                ErrorEvent{
                    make_error_code(
                        hl::http_error::UnableToConnect ),
                    ss.str()
                });
        }
    }
    void HandleProxyConnectWrite(
            RacePreventer race_preventer,
            std::shared_ptr<asio::streambuf> /* request_buf */,
            const TimePoint start_time,
            const std::size_t bytes_transferred,
            const boost::system::error_code& err
        )
    {
        // Skip if cancelled due to timeout.
        if ( ! race_preventer.IsFirst() ) return;

        ClearAsyncTimeout();  // Must stop timeout timer.

        if (bw_analyser_)
        {
            bw_analyser_->RecordOutgoingBytes( bytes_transferred, start_time,
                sclock::now() );
        }

        if (!err)
        {
            std::shared_ptr<asio::streambuf> response =
                std::make_shared<asio::streambuf>();

            // Must prime timeout for async actions.
            auto race_preventer = SetAsyncTimeout("proxy read response",
                kProxyReadTimeout);

            if ( is_ssl_ )
            {
                asio::async_read_until(
                    socket_wrapper_->SslSocket()->next_layer(),
                    *response, "\r\n\r\n",
                    boost::bind(
                        &HttpRequestMachine_::HandleProxyHeaderRead,
                        shared_from_this(),
                        race_preventer,
                        response,
                        sclock::now(),
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error));
            }
            else
            {
                asio::async_read_until(
                    *socket_wrapper_->Socket(),
                    *response,
                    "\r\n\r\n",
                    boost::bind(
                        &HttpRequestMachine_::HandleProxyHeaderRead,
                        shared_from_this(),
                        race_preventer,
                        response,
                        sclock::now(),
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error));
            }
        }
        else
        {
            std::stringstream ss;
            ss << "Failure connecting to proxy.";
            if ( is_ssl_ && https_proxy_ )
            {
                ss << " Proxy: " << https_proxy_->host;
                ss << ":" << https_proxy_->port;
            }
            else if ( ! is_ssl_ && http_proxy_ )
            {
                ss << " Proxy: " << http_proxy_->host;
                ss << ":" << http_proxy_->port;
            }

            ss << " Error: " << err.message();
            ProcessEvent(
                ErrorEvent{
                    make_error_code(
                        hl::http_error::UnableToConnectToProxy ),
                    ss.str()
                });
        }
    }
    void HandleProxyHeaderRead(
            RacePreventer race_preventer,
            std::shared_ptr<asio::streambuf> headers_buf,
            const TimePoint start_time,
            const std::size_t bytes_transferred,
            const boost::system::error_code& err
        )
    {
        // Skip if cancelled due to timeout.
        if ( ! race_preventer.IsFirst() ) return;

        ClearAsyncTimeout();  // Must stop timeout timer.

        if (bw_analyser_)
        {
            bw_analyser_->RecordIncomingBytes( bytes_transferred, start_time,
                sclock::now() );
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
                ProcessEvent(
                    ErrorEvent{
                        make_error_code(
                            hl::http_error::ProxyProtocolFailure ),
                        ss.str()
                    });
                return;
            }

            if ( status_code == 200 )
            {
                ProcessEvent(ConnectedEvent{});
            }
            else
            {
                std::stringstream ss;
                ss << "Protocol error while parsing proxy headers.";
                ss << " HTTP Status: " << status_code;
                ss << " " << status_message;
                ProcessEvent(
                    ErrorEvent{
                        make_error_code(
                            hl::http_error::ProxyProtocolFailure ),
                        ss.str()
                    });
            }
        }
        else
        {
            std::stringstream ss;
            ss << "Failure reading from proxy.";
            if ( is_ssl_ && https_proxy_ )
            {
                ss << " Proxy: " << https_proxy_->host;
                ss << ":" << https_proxy_->port;
            }
            else if ( ! is_ssl_ && http_proxy_ )
            {
                ss << " Proxy: " << http_proxy_->host;
                ss << ":" << http_proxy_->port;
            }

            ss << " Error: " << err.message();
            ProcessEvent(
                ErrorEvent{
                    make_error_code(
                        hl::http_error::ProxyProtocolFailure ),
                    ss.str()
                });
        }
    }
    void HandleHeaderWrite(
            RacePreventer race_preventer,
            std::shared_ptr<asio::streambuf> /* request_buf */,
            const TimePoint start_time,
            const std::size_t bytes_transferred,
            const boost::system::error_code& err
        )
    {
        // Skip if cancelled due to timeout.
        if ( ! race_preventer.IsFirst() ) return;

        ClearAsyncTimeout();  // Must stop timeout timer.

        if (bw_analyser_)
        {
            bw_analyser_->RecordOutgoingBytes( bytes_transferred, start_time,
                sclock::now() );
        }

        if (!err)
        {
            ProcessEvent(HeadersWrittenEvent{});
        }
        else
        {
            std::stringstream ss;
            ss << "Failure while writing headers url(" << url_ << ").";
            ss << " Error: " << err.message();
            ProcessEvent(
                ErrorEvent{
                    make_error_code(
                        hl::http_error::WriteFailure ),
                    ss.str()
                });
        }
    }
    void HandlePostWrite(
            RacePreventer race_preventer,
            hl::SharedBuffer::Pointer /* post_data */,  // keepalive
            const TimePoint start_time,
            const std::size_t bytes_transferred,
            const boost::system::error_code& err
        )
    {
        // Skip if cancelled due to timeout.
        if ( ! race_preventer.IsFirst() ) return;

        ClearAsyncTimeout();  // Must stop timeout timer.

        if (bw_analyser_)
        {
            bw_analyser_->RecordOutgoingBytes( bytes_transferred, start_time,
                sclock::now() );
        }

        if (!err)
        {
            if (post_interface_)
            {
                PostViaInterface( start_time, sclock::now() );
            }
            else
                ProcessEvent(PostSent{});
        }
        else
        {
            std::stringstream ss;
            ss << "Failure writing POST. URL: " << url_;
            ss << " Error: " << err.message();
            ProcessEvent(
                ErrorEvent{
                    make_error_code(
                        hl::http_error::WriteFailure ),
                    ss.str()
                });
        }
    }
    void HandleHeaderRead(
            RacePreventer race_preventer,
            const TimePoint start_time,
            const std::size_t bytes_transferred,
            const boost::system::error_code& err
        )
    {
        // Skip if cancelled due to timeout.
        if ( ! race_preventer.IsFirst() ) return;

        ClearAsyncTimeout();  // Must stop timeout timer.

        if (bw_analyser_)
        {
            bw_analyser_->RecordIncomingBytes( read_buffer_.size(), start_time,
                sclock::now() );
        }

        if (!err)
        {
            HeadersReadEvent evt;
            {
                // Copy headers.

                asio::streambuf::const_buffers_type bufs =
                    read_buffer_.data();

                evt.raw_headers = std::string(
                        asio::buffers_begin(bufs),
                        ( asio::buffers_begin(bufs)
                            + bytes_transferred )
                    );
            }

            std::istream response_stream(&read_buffer_);
            response_stream >> evt.http_version;
            response_stream >> evt.status_code;
            std::getline(response_stream, evt.status_message);

            if ( ! response_stream
                    || evt.http_version.substr(0, 5) != "HTTP/")
            {
                std::stringstream ss;
                ss << "Protocol error while parsing headers("
                    << url_ << ").";
                ProcessEvent(
                    ErrorEvent{
                        make_error_code(
                            hl::http_error::UnparsableHeaders ),
                        ss.str()
                    });
                return;
            }

            std::string line;
            std::string last_header_name;
            while (std::getline(response_stream, line) && line != "\r")
            {
                // Debug
                // std::cout << line << std::endl;

                boost::trim_right(line);

                if ( line.empty() ) continue;

                // HTTP headers can be split up inbetween lines.
                // http://www.w3.org/Protocols/rfc2616/rfc2616-sec2.html#sec2.2
                if ( line[0] == ' ' || line[0] == '\t' )
                {
                    if ( last_header_name.empty() )
                    {
                        std::stringstream ss;
                        ss << "Failure while reading headers url("
                            << url_ << ").";
                        ss << " Badly formatted headers.";
                        ProcessEvent(
                            ErrorEvent{
                                make_error_code(
                                    hl::http_error::UnparsableHeaders ),
                                ss.str()
                            });
                        return;
                    }

                    // This is a continuation of previous line.
                    auto it = evt.headers.find(last_header_name);
                    it->second += " ";
                    boost::trim(line);
                    it->second += line;

                    continue;
                }

                boost::iterator_range<std::string::iterator> result =
                    boost::find_first(line, ":");

                if ( ! result.empty() )
                {
                    std::string header_name = std::string(
                            line.begin(), result.begin());

                    std::string header_value = std::string(
                            result.end(), line.end());

                    boost::trim(header_name);
                    boost::to_lower(header_name);

                    boost::trim(header_value);

                    evt.headers.emplace(header_name, header_value);

                    // Record the last header name in case the next line is
                    // extended.
                    last_header_name.swap(header_name);
                }
            }

            {
                auto it = evt.headers.find("content-length");
                if ( it != evt.headers.end() )
                {
                    try {
                        evt.content_length = boost::lexical_cast<uint64_t>(
                                it->second);
                    } catch(boost::bad_lexical_cast &) {
                        std::stringstream ss;
                        ss << "Failure while parsing headers url("
                            << url_ << ").";
                        ss << " Invalid Content-Length: " << it->second;
                        ProcessEvent(
                            ErrorEvent{
                                make_error_code(
                                    hl::http_error::UnparsableHeaders ),
                                ss.str()
                            });
                        return;
                    }
                }
            }

            ProcessEvent(evt);  // Send HeadersReadEvent
        }
        else
        {
            std::stringstream ss;
            ss << "Failure while reading headers url(" << url_ << ").";
            ss << " Error: " << err.message();
            ProcessEvent(
                ErrorEvent{
                    make_error_code(
                        hl::http_error::ReadFailure ),
                    ss.str()
                });
        }
    }
    void HandleContentChunkSizeRead(
            RacePreventer race_preventer,
            uint64_t output_bytes_consumed,  // What we have sent to user.
            const int te,
            const int ce,
            const TimePoint start_time,
            Duration content_chunk_read_duration,
            const std::size_t bytes_transferred,
            const boost::system::error_code& err
        )
    {
        // Skip if cancelled due to timeout.
        if ( ! race_preventer.IsFirst() ) return;

        TimePoint now = sclock::now();
        ClearAsyncTimeout();  // Must stop timeout timer.

        if (bw_analyser_)
        {
            bw_analyser_->RecordIncomingBytes( bytes_transferred, start_time,
                now );
        }

        if ( !err || err == asio::error::eof )
        {
            std::string chunk_size_as_hex;
            std::istream response_stream(&read_buffer_);
            std::getline(response_stream, chunk_size_as_hex);

            uint64_t chunk_size = 0;
            try
            {
                chunk_size = mf::utils::str_to_uint64(chunk_size_as_hex, 16);
            }
            catch(std::invalid_argument & err)
            {
                std::stringstream ss;
                ss << "Failure while parsing chunk: " << err.what();
                ss << " Chunk: " << chunk_size_as_hex;
                ProcessEvent(
                    ErrorEvent{
                        make_error_code(
                            hl::http_error::ReadFailure ),
                        ss.str()
                    });
                return;
            }
            catch(std::out_of_range & err)
            {
                std::stringstream ss;
                ss << "Failure while parsing chunk: " << err.what();
                ss << " Chunk: " << chunk_size_as_hex;
                ProcessEvent(
                    ErrorEvent{
                        make_error_code(
                            hl::http_error::ReadFailure ),
                        ss.str()
                    });
                return;
            }

            // Debug
            // std::cout << "Got chunk size: " << chunk_size << std::endl;

            if ( chunk_size == 0 )  // Last chunk is 0 bytes long.
            {
                HandleCompleteAllChunks(ce);
            }
            else
            {
                HandleNextChunk( output_bytes_consumed, te, ce, start_time,
                    content_chunk_read_duration, chunk_size);
            }
        }
        else
        {
            std::stringstream ss;
            ss << "Failure while reading content.";
            ss << " Url: " << url_;
            ss << " Error: " << err.message();
            ProcessEvent(
                ErrorEvent{
                    make_error_code(
                        hl::http_error::ReadFailure ),
                    ss.str()
                });
        }
    }
    void HandleNextChunk(
            uint64_t output_bytes_consumed,  // What we have sent to user.
            const int te,
            const int ce,
            const TimePoint start_time,
            Duration content_chunk_read_duration,
            uint64_t chunk_size
        )
    {
        TimePoint now = sclock::now();

        // Determine how much more to read, or if the buffer
        // already contains enough data to continue.
        uint64_t left_to_read = chunk_size+2;
        if ( read_buffer_.size() > left_to_read )
            left_to_read = 0;
        else
            left_to_read -= read_buffer_.size();

        // Delay behavior:
        auto total_duration = AsDuration(content_chunk_read_duration +
            (now - start_time));
        SetTransactionDelayTimer( now, total_duration );
        std::function<void()> strand_wrapped = std::bind(
            &HttpRequestMachine_::HandleContentChunkSizeReadDelay,
            shared_from_this(),
            left_to_read,
            output_bytes_consumed,
            chunk_size,
            te,
            ce
        );
        transmission_delay_timer_.async_wait(
            event_strand_.wrap(
                boost::bind(
                    &HttpRequestMachine_::SetTransactionDelayTimerWrapper,
                    shared_from_this(),
                    strand_wrapped,
                    asio::placeholders::error
                )));
    }
    void HandleCompleteAllChunks(
            const int ce
        )
    {
        if ( ce & CE_Gzip )
        {
            std::shared_ptr<VectorBuffer> return_buffer(
                new VectorBuffer() );

            filter_buf_.push( gzipped_buffer_ );

            try {
                boost::iostreams::copy(
                    filter_buf_,
                    std::back_inserter(return_buffer->buffer));
            }
            catch( boost::iostreams::gzip_error & err )
            {
                std::stringstream ss;
                ss << "Compression failure.";
                ss << " Error: " << err.what();
                switch (err.error())
                {
                    case boost::iostreams::gzip::zlib_error:
                        ss << " GZip error: zlib_error";
                        break;
                    case boost::iostreams::gzip::bad_crc:
                        ss << " GZip error: bad_crc";
                        break;
                    case boost::iostreams::gzip::bad_length:
                        ss << " GZip error: bad_length";
                        break;
                    case boost::iostreams::gzip::bad_header:
                        ss << " GZip error: bad_header";
                        break;
                    case boost::iostreams::gzip::bad_footer:
                        ss << " GZip error: bad_footer";
                        break;
                    default:
                        ss << " GZip error: " << err.error();
                        break;
                }
                ss << " Zlib error: " << err.zlib_error_code();
                ProcessEvent(ErrorEvent{
                    make_error_code(
                        hl::http_error::CompressionFailure ),
                    ss.str()
                    });
                return;
            }
            catch( std::exception & err )
            {
                std::stringstream ss;
                ss << "Compression failure.";
                ss << " Error: " << err.what();
                ProcessEvent(ErrorEvent{
                    make_error_code(
                        hl::http_error::CompressionFailure ),
                    ss.str()
                    });
                //assert(!"GZip compression error from chunks");
                return;
            }

            std::shared_ptr<hl::RequestResponseInterface> iface(
                callback_);

            uint64_t filter_buf_consumed = filter_buf_consumed_;
            callback_io_service_->dispatch(
                [iface, return_buffer, filter_buf_consumed]()
                {
                    iface->ResponseContentReceived(
                        filter_buf_consumed, return_buffer );
                }
            );

            // New byte counts
            filter_buf_consumed_ += return_buffer->buffer.size();
        }

        ProcessEvent(ContentReadEvent{});
    }
    void HandleContentChunkSizeReadDelay(
        uint64_t left_to_read,
        uint64_t output_bytes_consumed,
        uint64_t chunk_size,
        const int te,
        const int ce
        )
    {
        // Ensure a cancellation doesn't mess up the state due to async
        // timer.
        if ( transmission_delay_timer_enabled_ )
        {
            // Must prime timeout for async actions.
            auto race_preventer = SetAsyncTimeout("read response content 3",
                timeout_seconds_);

            if ( is_ssl_ )
            {
                // transfer_exactly required as we want the whole chunk
                asio::async_read(*socket_wrapper_->SslSocket(), read_buffer_,
                    asio::transfer_exactly(left_to_read),
                    boost::bind(
                        &HttpRequestMachine_::HandleContentChunkRead,  // NOLINT
                        shared_from_this(),
                        race_preventer,
                        output_bytes_consumed,
                        chunk_size,
                        te, ce,
                        sclock::now(),
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error
                    )
                );
            }
            else
            {
                asio::async_read(*socket_wrapper_->Socket(), read_buffer_,
                    asio::transfer_exactly(left_to_read),
                    boost::bind(
                        &HttpRequestMachine_::HandleContentChunkRead,  // NOLINT
                        shared_from_this(),
                        race_preventer,
                        output_bytes_consumed,
                        chunk_size,
                        te, ce,
                        sclock::now(),
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error
                    )
                );
            }
        }
    }
    void HandleContentChunkRead(
            RacePreventer race_preventer,
            uint64_t output_bytes_consumed,  // What we have sent to user.
            uint64_t chunk_size,
            const int te,
            const int ce,
            const TimePoint start_time,
            const std::size_t bytes_transferred,
            const boost::system::error_code& err
        )
    {
        // Skip if cancelled due to timeout.
        if ( ! race_preventer.IsFirst() ) return;

        ClearAsyncTimeout();  // Must stop timeout timer.

        if (bw_analyser_)
        {
            bw_analyser_->RecordIncomingBytes( bytes_transferred, start_time,
                sclock::now() );
        }

        auto content_chunk_read_duration = AsDuration(sclock::now() -
            start_time);
        bool eof = false;

        if ( err == asio::error::eof )
            eof = true;
        else if ( is_ssl_ && err.message() == "short read" )
        {
            // SSL doesn't return EOF when it ends.
            eof = true;
        }

        if ( !err || eof )
        {
            if ( ce & CE_Gzip )
            {
                boost::iostreams::copy(
                        boost::iostreams::restrict(
                            read_buffer_, 0, chunk_size ),
                        gzipped_buffer_ );

                output_bytes_consumed += chunk_size;
            }
            else
            {
                // Non gzip buffer passing.
                std::istream post_data_stream(&read_buffer_);
                std::unique_ptr<uint8_t[]> data( new uint8_t[chunk_size] );
                post_data_stream.read( reinterpret_cast<char*>(data.get()),
                    chunk_size );

                std::shared_ptr<hl::BufferInterface> return_buffer(
                        new HttpRequestBuffer(
                            std::move(data),
                            chunk_size )
                        );
                std::shared_ptr<hl::RequestResponseInterface> iface(
                        callback_);
                callback_io_service_->dispatch(
                        [iface, return_buffer, output_bytes_consumed]()
                        {
                            iface->ResponseContentReceived(
                                output_bytes_consumed, return_buffer );
                        }
                        );

                // New byte counts
                output_bytes_consumed += chunk_size;
            }

            read_buffer_.consume( 2 );  // +2 for \r\n after chunk.

            // Must prime timeout for async actions.
            auto race_preventer = SetAsyncTimeout("read response content 4",
                timeout_seconds_);

            if ( is_ssl_ )
            {
                asio::async_read_until(
                    *socket_wrapper_->SslSocket(),
                    read_buffer_,
                    "\r\n",
                    boost::bind(
                        &HttpRequestMachine_::HandleContentChunkSizeRead,
                        shared_from_this(),
                        race_preventer,
                        output_bytes_consumed,
                        te, ce,
                        sclock::now(),
                        content_chunk_read_duration,
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error
                        )
                    );
            }
            else
            {
                asio::async_read_until(
                    *socket_wrapper_->Socket(),
                    read_buffer_,
                    "\r\n",
                    boost::bind(
                        &HttpRequestMachine_::HandleContentChunkSizeRead,
                        shared_from_this(),
                        race_preventer,
                        output_bytes_consumed,
                        te, ce,
                        sclock::now(),
                        content_chunk_read_duration,
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error
                        )
                    );
            }
        }
        else
        {
            std::stringstream ss;
            ss << "Failure while reading chunked content.";
            ss << " Url: " << url_;
            ss << " Error: " << err.message();
            ProcessEvent(
                ErrorEvent{
                    make_error_code(
                        hl::http_error::ReadFailure ),
                    ss.str()
                });
        }
    }
    void HandleContentRead(
            RacePreventer race_preventer,
            const std::size_t total_previously_read,
            const int te,
            const int ce,
            const TimePoint start_time,
            const std::size_t bytes_previously_transferred,
            const std::size_t bytes_transferred,
            const boost::system::error_code& err
        )
    {
        // Skip if cancelled due to timeout.
        if ( ! race_preventer.IsFirst() ) return;

        const std::size_t bytes_to_process =
            bytes_previously_transferred + bytes_transferred;

        ClearAsyncTimeout();  // Must stop timeout timer.

        if (bw_analyser_)
        {
            bw_analyser_->RecordIncomingBytes( bytes_previously_transferred,
                start_time, sclock::now() );
        }

        bool eof = false;

        if ( err == asio::error::eof )
            eof = true;
        else if ( is_ssl_ && err.message() == "short read" )
        {
            // SSL doesn't return EOF when it ends.
            eof = true;
        }

        if ( !err || eof )
        {
            bool read_complete = false;

            const std::size_t total_read =
                total_previously_read + bytes_to_process;

            if ( ! ( ce & CE_Gzip ) )
            {
                // Non gzip buffer passing.
                std::istream post_data_stream(&read_buffer_);
                std::unique_ptr<uint8_t[]> data(new uint8_t[bytes_to_process]);
                post_data_stream.read( reinterpret_cast<char*>(data.get()),
                    bytes_to_process );
                std::shared_ptr<hl::BufferInterface> return_buffer(
                        new HttpRequestBuffer(
                            std::move(data),
                            bytes_to_process )
                        );

                std::shared_ptr<hl::RequestResponseInterface> iface(
                        callback_);

                callback_io_service_->dispatch(
                        [iface, return_buffer, total_previously_read]()
                        {
                            iface->ResponseContentReceived(
                                total_previously_read, return_buffer );
                        }
                    );
            }

            // Also handle content-length.
            if ( te & TE_ContentLength )
            {
                if ( read_headers_.content_length == total_read )
                {
                    read_complete = true;
                }
                else if ( read_headers_.content_length < total_read )
                {
                    std::stringstream ss;
                    ss << "Failure while reading content.";
                    ss << " Url: " << url_;
                    ss << " Error: Exceeded content length.";
                    // ss << " Total read: " << total_read;
                    // ss << " Content length: "
                    //    << read_headers_.content_length;

                    ProcessEvent(
                        ErrorEvent{
                            make_error_code(
                                hl::http_error::ReadFailure ),
                            ss.str()
                        });
                    return;
                }
            }
            else if ( eof )
                read_complete = true;

            if ( read_complete )
            {
                if ( ce & CE_Gzip )
                {
                    filter_buf_.push( read_buffer_ );

                    std::shared_ptr<VectorBuffer> return_buffer(
                            new VectorBuffer() );

                    try {
                        boost::iostreams::copy(
                                filter_buf_,
                                std::back_inserter(return_buffer->buffer));
                    }
                    catch( std::exception & err )
                    {
                        std::stringstream ss;
                        ss << "Compression failure.";
                        ss << " Error: " << err.what();
                        ProcessEvent(
                            ErrorEvent{
                                make_error_code(
                                    hl::http_error::ReadFailure ),
                                ss.str()
                            });
                        //assert(!"GZip compression error from content");
                        return;
                    }

                    std::shared_ptr<hl::RequestResponseInterface> iface(
                            callback_);

                    uint64_t filter_buf_consumed = filter_buf_consumed_;

                    callback_io_service_->dispatch(
                            [iface, return_buffer, filter_buf_consumed]()
                            {
                                iface->ResponseContentReceived(
                                    filter_buf_consumed, return_buffer );
                            }
                        );

                    // New byte counts
                    filter_buf_consumed_ += return_buffer->buffer.size();
                }

                ProcessEvent(ContentReadEvent{});
            }
            else
            {
                uint64_t max_read_size = kMaxUnknownReadLength;
                if ( te & TE_ContentLength )
                {
                    max_read_size = std::min(
                            read_headers_.content_length - total_read,
                            kMaxUnknownReadLength
                        );
                }

                // Delay behavior:
                SetTransactionDelayTimer( start_time, sclock::now() );
                std::function<void()> strand_wrapped = std::bind(
                    &HttpRequestMachine_::HandleContentReadDelayCallback,
                    shared_from_this(),
                    max_read_size,
                    total_read,
                    te,
                    ce
                );
                transmission_delay_timer_.async_wait(
                    event_strand_.wrap(
                        boost::bind(
                            &HttpRequestMachine_::SetTransactionDelayTimerWrapper,
                            shared_from_this(),
                            strand_wrapped,
                            asio::placeholders::error
                        )));
            }
        }
        else
        {
            std::stringstream ss;
            ss << "Failure while reading content.";
            ss << " Url: " << url_;
            ss << " Error: " << err.message();
            ProcessEvent(
                ErrorEvent{
                    make_error_code(
                        hl::http_error::ReadFailure ),
                    ss.str()
                });
        }
    }
    void HandleContentReadDelayCallback(
            const uint64_t max_read_size,
            const std::size_t total_read,
            const int te,
            const int ce
        )
    {
        // Ensure a cancellation doesn't mess up the state due to async
        // timer.
        if ( transmission_delay_timer_enabled_ )
        {
            transmission_delay_timer_enabled_ = false;

            // Must prime timeout for async actions.
            auto race_preventer = SetAsyncTimeout("read response content 5",
                timeout_seconds_);

            if ( is_ssl_ )
            {
                /* hjones note: asio::transfer_exactly might not be needed here
                 * as SSL may take care of the proper read size for us */
                asio::async_read(*socket_wrapper_->SslSocket(), read_buffer_,
                    asio::transfer_exactly(max_read_size),
                    boost::bind(
                        &HttpRequestMachine_::HandleContentRead,
                        shared_from_this(),
                        race_preventer,
                        total_read,
                        te, ce,
                        sclock::now(),
                        0,
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error
                    )
                );
            }
            else
            {
                asio::async_read(*socket_wrapper_->Socket(), read_buffer_,
                    asio::transfer_exactly(max_read_size),
                    boost::bind(
                        &HttpRequestMachine_::HandleContentRead,
                        shared_from_this(),
                        race_preventer,
                        total_read,
                        te, ce,
                        sclock::now(),
                        0,
                        asio::placeholders::bytes_transferred,
                        asio::placeholders::error
                    )
                );
            }
        }
    }

    // Helper actions
    void SetTransactionDelayTimer(
            TimePoint last_write_start,
            TimePoint last_write_end
        )
    {
        const auto length = AsDuration(last_write_end - last_write_start);
        SetTransactionDelayTimer( last_write_end, length );
    }
    void SetTransactionDelayTimer(
        TimePoint last_write_end,
        Duration length
        )
    {
        TimePoint expire_time = last_write_end;

        using std::chrono::duration_cast;
        using std::chrono::microseconds;

        uint32_t mss = duration_cast<microseconds>(length).count();

        // std::cout << "Total microseconds: " << mss << std::endl;

        // Get microseconds to delay
        mss *= delay_multiplier_;

        // std::cout << "Total delay microseconds: " << mss
        //     << std::endl;

        expire_time += microseconds(mss);

        transmission_delay_timer_enabled_ = true;
        transmission_delay_timer_.expires_at( expire_time );
    }
    void SetTransactionDelayTimerWrapper(
            std::function<void()> bind_function,
            const boost::system::error_code& err
        )
    {
        // Since we can't set a timer to a strand, and we must check the
        // enabled flag in the strand for thread safety, we use the strand
        // here.
        if ( ! err )
        {
            event_strand_.dispatch(
                bind_function );
        }
    }
    void ClearAsyncTimeout()
    {
        // Change the id for when handler and timeout end up on stack
        // together.
        ++timeout_id_;

        timer_.cancel();
    }
    RacePreventer SetAsyncTimeout(std::string reason, uint32_t timeout_seconds)
    {
        RacePreventer race_preventer(socket_wrapper_.get());

        // Change the id for when handler and timeout end up on stack
        // together.
        ++timeout_id_;

        timer_.cancel();

        // Keep reason for timeout for debug message.
        timeout_reason_ = reason;

        timer_.expires_from_now(std::chrono::seconds(timeout_seconds));
        timer_.async_wait(
            event_strand_.wrap(
                boost::bind(
                    &HttpRequestMachine_::HandleAsyncTimeout,
                    shared_from_this(),
                    race_preventer,
                    timeout_id_,
                    asio::placeholders::error
                )));
        return race_preventer;
    }
    void Disconnect()
    {
        // Close connection
        if (socket_wrapper_)
        {
            socket_wrapper_->Cancel();
            socket_wrapper_.reset();
        }
    }
    void SetHeader(const std::string & name, const std::string & value)
    {
        using CI = hl::HttpRequest::HeaderContainer::value_type;
        auto it = std::find_if(
                headers_.begin(),
                headers_.end(),
                [&name](const CI & pair)
                {
                    return boost::iequals(pair.first, name);
                }
            );
        if ( it != headers_.end() )
        {
            it->second = value;
        }
        else
        {
            headers_.emplace_back(name, value);
        }
    }

    typedef HttpRequestMachine_ m;  // makes transition table cleaner

    // Transition table for HttpRequestMachine
    struct transition_table : mpl::vector<
        //    Start           Event                  Next            Action                         Guard                     // NOLINT
        //  +---------------+-----------------------+---------------+------------------------------+----------------------+   // NOLINT
      a_row < Unstarted     , ConfigEvent           , Unstarted     , &m::ConfigEventAction                               >,  // NOLINT
      a_row < Unstarted     , StartEvent            , Initializing  , &m::InitializeAction                                >,  // NOLINT
        //  +---------------+-----------------------+---------------+------------------------------+----------------------+   // NOLINT
      a_row < Initializing  , InitializedEvent      , Resolve       , &m::ResolveHostAction                               >,  // NOLINT
      a_row < Initializing  , ErrorEvent            , Error         , &m::VerifyErrorAction                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+------------------------------+----------------------+   // NOLINT
      a_row < Resolve       , ResolvedEvent         , Connect       , &m::ConnectAction                                   >,  // NOLINT
      a_row < Resolve       , ErrorEvent            , Error         , &m::VerifyErrorAction                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+------------------------------+----------------------+   // NOLINT
        row < Connect       , ConnectedEvent        , SendHeader    , &m::SendHeaderAction         , &m::IsNotSsl         >,  // NOLINT
        row < Connect       , ConnectedEvent        , ProxyConnect  , &m::ProxyConnectAction       , &m::IsSslAndProxy    >,  // NOLINT
        row < Connect       , ConnectedEvent        , SSLHandshake  , &m::SSLHandshakeAction       , &m::IsSslAndNotProxy >,  // NOLINT
      a_row < Connect       , ErrorEvent            , Error         , &m::VerifyErrorAction                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+------------------------------+----------------------+   // NOLINT
        row < ProxyConnect  , ConnectedEvent        , SendHeader    , &m::SendHeaderAction         , &m::IsNotSsl         >,  // NOLINT
        row < ProxyConnect  , ConnectedEvent        , SSLHandshake  , &m::SSLHandshakeAction       , &m::IsSsl            >,  // NOLINT
      a_row < ProxyConnect  , ErrorEvent            , Error         , &m::VerifyErrorAction                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+------------------------------+----------------------+   // NOLINT
      a_row < SSLHandshake  , HandshakeEvent        , SendHeader    , &m::SendHeaderAction                                >,  // NOLINT
      a_row < SSLHandshake  , ErrorEvent            , Error         , &m::VerifyErrorAction                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+------------------------------+----------------------+   // NOLINT
        row < SendHeader    , HeadersWrittenEvent   , SendPost      , &m::SendPostAction           , &m::HasPost          >,  // NOLINT
        row < SendHeader    , HeadersWrittenEvent   , ReadHeaders   , &m::ReadHeaderAction         , &m::HasNoPost        >,  // NOLINT
      a_row < SendHeader    , ErrorEvent            , Error         , &m::VerifyErrorAction                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+------------------------------+----------------------+   // NOLINT
      a_row < SendPost      , PostSent              , ReadHeaders   , &m::ReadHeaderAction                                >,  // NOLINT
      a_row < SendPost      , ErrorEvent            , Error         , &m::VerifyErrorAction                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+------------------------------+----------------------+   // NOLINT
      a_row < ReadHeaders   , HeadersReadEvent      , ParseHeaders  , &m::ParseHeadersAction                              >,  // NOLINT
      a_row < ReadHeaders   , ErrorEvent            , Error         , &m::VerifyErrorAction                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+------------------------------+----------------------+   // NOLINT
      a_row < ParseHeaders  , HeadersParsedEvent    , ReadContent   , &m::ReadContentAction                               >,  // NOLINT
      a_row < ParseHeaders  , RedirectEvent         , Redirect      , &m::RedirectAction                                  >,  // NOLINT
      a_row < ParseHeaders  , ErrorEvent            , Error         , &m::VerifyErrorAction                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+------------------------------+----------------------+   // NOLINT
      a_row < Redirect      , RedirectedEvent       , Initializing  , &m::InitializeAction                                >,  // NOLINT
      a_row < Redirect      , ErrorEvent            , Error         , &m::VerifyErrorAction                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+------------------------------+----------------------+   // NOLINT
       _row < ReadContent   , ContentReadEvent      , Complete                                                            >,  // NOLINT
      a_row < ReadContent   , ErrorEvent            , Error         , &m::VerifyErrorAction                               >,  // NOLINT
        //  +---------------+-----------------------+---------------+------------------------------+----------------------+   // NOLINT
      a_row < Error         , RestartEvent          , Initializing  , &m::InitializeAction                                >,  // NOLINT
       _row < Error         , ErrorEvent            , FinalError                                                          >   // NOLINT
        //  +---------------+-----------------------+---------------+------------------------------+----------------------+   // NOLINT
    > {};

    // Replaces the default no-transition response. Use this if you don't
    // want the machine to assert on an unexpected event.
    template <typename FSM, typename Event>
    void no_transition(Event const& e, FSM&, int state)
    {
        std::cerr << "no transition from state " << state
            << " on event " << typeid(e).name() << std::endl;
        assert(!"improper transition in http state machine");
    }

protected:
#if ! defined(NDEBUG)
    // Counter to keep track of number of HttpRequests
    static boost::atomic<int> request_count_;
#endif

    HttpConfig::ConstPointer http_config_;

    // IOService for work.
    asio::io_service * work_io_service_;

    // Strand for events.
    asio::io_service::strand event_strand_;

    // Track bandwidth
    hl::BandwidthAnalyserInterface::Pointer bw_analyser_;

    const std::chrono::time_point<std::chrono::steady_clock>
        request_creation_time_;

    // Use these to delay transmission of data for QOS.
    boost::asio::steady_timer transmission_delay_timer_;
    bool transmission_delay_timer_enabled_;

    // Timeout timer
    boost::asio::steady_timer timer_;
    uint32_t timeout_seconds_;
    uint32_t timeout_id_;
    std::string timeout_reason_;

    // Call to request user action.
    std::shared_ptr<hl::RequestResponseInterface> callback_;

    // IOService for callbacks.
    asio::io_service * callback_io_service_;

    // Asio internals
    asio::ip::tcp::resolver resolver_;

    hl::RedirectPolicy redirect_policy_;

    bool is_ssl_;
    boost::asio::ssl::context * ssl_ctx_;

    std::shared_ptr<SocketWrapper> socket_wrapper_;

    const asio::ssl::verify_mode ssl_verify_mode_;

    // Send data.
    std::string request_method_;
    std::string url_;
    const std::string original_url_;
    std::unique_ptr<hl::Url> parsed_url_;

    hl::SharedBuffer::Pointer post_data_;
    std::shared_ptr<hl::PostDataPipeInterface> post_interface_;
    size_t post_interface_size_;
    size_t post_interface_read_bytes_;

    hl::HttpRequest::HeaderContainer headers_;
    HeadersReadEvent read_headers_;

    boost::optional<hl::Proxy> http_proxy_;
    boost::optional<hl::Proxy> https_proxy_;

    asio::streambuf read_buffer_;
    asio::streambuf gzipped_buffer_;

    boost::iostreams::filtering_streambuf<boost::iostreams::input>
        filter_buf_;
    std::size_t filter_buf_consumed_;

    const float delay_multiplier_;
};


}  // namespace detail
}  // namespace http
}  // namespace mf

