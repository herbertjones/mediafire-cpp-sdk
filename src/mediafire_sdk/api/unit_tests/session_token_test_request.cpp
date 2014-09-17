/**
 * @file session_token_test_request.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "session_token_test_request.hpp"

#include <iostream>
#include <string>

namespace au = mf::api::ut;
namespace asio = boost::asio;

namespace status_string {

const std::string ok =
    "HTTP/1.0 200 OK\r\n";
const std::string created =
    "HTTP/1.0 201 Created\r\n";
const std::string accepted =
    "HTTP/1.0 202 Accepted\r\n";
const std::string no_content =
    "HTTP/1.0 204 No Content\r\n";
const std::string multiple_choices =
    "HTTP/1.0 300 Multiple Choices\r\n";
const std::string moved_permanently =
    "HTTP/1.0 301 Moved Permanently\r\n";
const std::string moved_temporarily =
    "HTTP/1.0 302 Moved Temporarily\r\n";
const std::string not_modified =
    "HTTP/1.0 304 Not Modified\r\n";
const std::string bad_request =
    "HTTP/1.0 400 Bad Request\r\n";
const std::string unauthorized =
    "HTTP/1.0 401 Unauthorized\r\n";
const std::string forbidden =
    "HTTP/1.0 403 Forbidden\r\n";
const std::string not_found =
    "HTTP/1.0 404 Not Found\r\n";
const std::string internal_server_error =
    "HTTP/1.0 500 Internal Server Error\r\n";
const std::string not_implemented =
    "HTTP/1.0 501 Not Implemented\r\n";
const std::string bad_gateway =
    "HTTP/1.0 502 Bad Gateway\r\n";
const std::string service_unavailable =
    "HTTP/1.0 503 Service Unavailable\r\n";

const std::string & FromCode(au::HttpStatusCode code)
{
    switch (code)
    {
        case au::HttpStatusCode::ok:
            return ok;
        case au::HttpStatusCode::created:
            return created;
        case au::HttpStatusCode::accepted:
            return accepted;
        case au::HttpStatusCode::no_content:
            return no_content;
        case au::HttpStatusCode::multiple_choices:
            return multiple_choices;
        case au::HttpStatusCode::moved_permanently:
            return moved_permanently;
        case au::HttpStatusCode::moved_temporarily:
            return moved_temporarily;
        case au::HttpStatusCode::not_modified:
            return not_modified;
        case au::HttpStatusCode::bad_request:
            return bad_request;
        case au::HttpStatusCode::unauthorized:
            return unauthorized;
        case au::HttpStatusCode::forbidden:
            return forbidden;
        case au::HttpStatusCode::not_found:
            return not_found;
        case au::HttpStatusCode::internal_server_error:
            return internal_server_error;
        case au::HttpStatusCode::not_implemented:
            return not_implemented;
        case au::HttpStatusCode::bad_gateway:
            return bad_gateway;
        case au::HttpStatusCode::service_unavailable:
            return service_unavailable;
        default:
            return internal_server_error;
    }
}

}  // namespace status_string

au::Request::Request(
        boost::asio::io_service * io_service_,
        boost::asio::ssl::context * ssl_ctx_,
        boost::asio::ip::tcp::socket socket_
) :
    io_service(io_service_),
    socket(std::move(socket_)),
    ssl_stream(socket, *ssl_ctx_)
{
}

au::Request::~Request()
{
}

void au::Request::Fail(
        HttpStatusCode status_code
    )
{
    auto self(shared_from_this());

    std::shared_ptr<boost::asio::streambuf> write_buffer_(
        std::make_shared<boost::asio::streambuf>() );

    std::ostream request_stream(write_buffer_.get());
    request_stream << status_string::FromCode(status_code);
    request_stream << "\r\n";

    asio::async_write(
        ssl_stream,
        *write_buffer_.get(),
        [this, self, write_buffer_](
                boost::system::error_code /* ec */,
                std::size_t /* bytes_transferred */
            )
        {
        });
}

void au::Request::Respond(
        HttpStatusCode status_code,
        const std::string & response
    )
{
    auto self(shared_from_this());

    std::shared_ptr<boost::asio::streambuf> write_buffer_(
        std::make_shared<boost::asio::streambuf>() );

    std::ostream request_stream(write_buffer_.get());
    request_stream << status_string::FromCode(status_code);
    request_stream << "Content-Length: " << response.size() << "\r\n";
    request_stream << "\r\n";
    request_stream << response;

    const std::string to_write([&write_buffer_]()
    {
        asio::streambuf::const_buffers_type bufs = write_buffer_->data();

        return std::string(
            asio::buffers_begin(bufs),
            ( asio::buffers_begin(bufs) + write_buffer_->size() )
        );
    }());

    asio::async_write(
        ssl_stream,
        *write_buffer_.get(),
        [this, self, write_buffer_](
                boost::system::error_code /* ec */,
                std::size_t /* bytes_transferred */
            )
        {
        });
}
