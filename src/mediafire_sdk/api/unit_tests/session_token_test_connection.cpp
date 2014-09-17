/**
 * @file session_token_test_connection.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "session_token_test_connection.hpp"

#include <iostream>
#include <string>
#include <vector>

#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "boost/algorithm/string/case_conv.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/bind.hpp"
#include "boost/lexical_cast.hpp"

#include "session_token_test.hpp"
#include "mediafire_sdk/utils/url_encode.hpp"

namespace au = mf::api::ut;
namespace asio = boost::asio;

au::SessionTokenTestConnection::SessionTokenTestConnection(
        boost::asio::io_service * io_service,
        boost::asio::ssl::context * ssl_ctx,
        boost::asio::ip::tcp::socket socket,
        PathHandlers * handlers
    ) :
    request_(
        new Request(
            io_service, ssl_ctx, std::move(socket) )),
    handlers_(handlers)
{
}

void au::SessionTokenTestConnection::Start()
{
    Handshake();
}

void au::SessionTokenTestConnection::Handshake()
{
    auto self(shared_from_this());

    request_->ssl_stream.async_handshake(
        boost::asio::ssl::stream_base::server,
        [this, self](
                boost::system::error_code ec
            )
        {
            if (ec)
            {
                std::cout << "Handshake: Got error: " << ec.message()
                    << std::endl;
                request_->Fail(HttpStatusCode::bad_request);
                return;
            }

            ReadHeaders();
        });
}

void au::SessionTokenTestConnection::ReadHeaders()
{
    auto self(shared_from_this());
    asio::async_read_until(
        request_->ssl_stream,
        request_->read_buffer_,
        "\r\n\r\n",
        [this, self](
                boost::system::error_code ec,
                std::size_t bytes_transferred
            )
        {
            if (ec)
            {
                std::cout << "ReadHeaders: Got error: " << ec.message()
                    << std::endl;
                request_->Fail(HttpStatusCode::bad_request);
                return;
            }

            ParseHeaders( bytes_transferred );
        });
}

void au::SessionTokenTestConnection::ParseHeaders(
        const std::size_t bytes_transferred
    )
{
    const std::string raw_headers([this, bytes_transferred]
    {
        asio::streambuf::const_buffers_type bufs =
            request_->read_buffer_.data();

        return std::string(
            asio::buffers_begin(bufs),
            ( asio::buffers_begin(bufs) + bytes_transferred )
        );
    }());

    std::string line;

    uint64_t content_length = 0;

    std::istream response_stream(&request_->read_buffer_);
    response_stream >> request_->http_method;
    response_stream >> request_->http_request_path;
    response_stream >> request_->http_version;
    std::getline(response_stream, line);

    if ( ! response_stream || request_->http_version.substr(0, 5) != "HTTP/")
    {
        std::cout << "Bad HTTP header." << std::endl;
        request_->Fail(HttpStatusCode::bad_request);
        return;
    }

    std::string last_header_name;
    while (std::getline(response_stream, line) && line != "\r")
    {
        boost::trim_right(line);

        if ( line.empty() ) continue;

        // HTTP headers can be split up inbetween lines.
        // http://www.w3.org/Protocols/rfc2616/rfc2616-sec2.html#sec2.2
        if ( line[0] == ' ' || line[0] == '\t' )
        {
            if ( last_header_name.empty() )
            {
                std::cout << "Bad headers." << std::endl;
                request_->Fail(HttpStatusCode::bad_request);
                return;
            }

            // This is a continuation of previous line.
            auto it = request_->headers.find(last_header_name);
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

            request_->headers.emplace(header_name, header_value);

            // Record the last header name in case the next line is
            // extended.
            last_header_name.swap(header_name);
        }
    }

    {
        auto it = request_->headers.find("content-length");
        if ( it != request_->headers.end() )
        {
            try {
                content_length = boost::lexical_cast<uint64_t>(
                    it->second);
            } catch(boost::bad_lexical_cast &) {
                std::cout << "Invalid content length." << std::endl;
                request_->Fail(HttpStatusCode::bad_request);
                return;
            }
        }
    }

    std::vector<std::string> request_path_parts;
    boost::split(
        request_path_parts,
        request_->http_request_path,
        boost::is_any_of("?"));

    if ( request_path_parts.size() > 2 )
    {
        std::cout << "Bad request path: " << request_->http_request_path
            << std::endl;
        request_->Fail(HttpStatusCode::bad_request);
        return;
    }

    request_->path = request_path_parts[0];

    auto handler_it = handlers_->find( request_->path );
    if ( handler_it == handlers_->end() )
    {
        std::cout << "Server error: no handler for path: " << request_->path
            << std::endl;
        request_->Fail(HttpStatusCode::not_found);
        return;
    }

    if ( request_path_parts.size() > 1 )
    {
        request_->query = request_path_parts[1];

        std::vector<std::string> query_parts;
        boost::split(
            query_parts,
            request_->query,
            boost::is_any_of("&"));

        for ( const std::string & part : query_parts )
        {
            std::vector<std::string> key_pair;
            boost::split(
                key_pair,
                part,
                boost::is_any_of("="));

            boost::optional<std::string> key = mf::utils::UrlUnencode(
                key_pair[0] );
            if ( ! key )
            {
                std::cout << "Got bad url data." << std::endl;
                request_->Fail(HttpStatusCode::bad_request);
                return;
            }

            if (key_pair.size() > 1)
            {
                boost::optional<std::string> value = mf::utils::UrlUnencode(
                    key_pair[1] );
                if ( ! value )
                {
                    std::cout << "Got bad url data." << std::endl;
                    request_->Fail(HttpStatusCode::bad_request);
                    return;
                }

                request_->query_parts[*key] = *value;
            }
            else
                request_->query_parts[*key] = "";
        }
    }

    if ( content_length > 0 )
    {
        auto self(shared_from_this());

        // Empty buffer for reuse
        request_->read_buffer_.consume( request_->read_buffer_.size() );

        asio::async_read(
            request_->ssl_stream,
            request_->read_buffer_,
            asio::transfer_exactly(content_length),
            boost::bind(
                &au::SessionTokenTestConnection::HandleContentRead,
                shared_from_this(),
                asio::placeholders::bytes_transferred,
                asio::placeholders::error
            )
        );
    }
    else
    {
        // Execute the handler.
        (handler_it->second)(std::move(request_));
    }
}

void au::SessionTokenTestConnection::HandleContentRead(
        const std::size_t bytes_transferred,
        const boost::system::error_code& ec
    )
{
    if (ec)
    {
        std::cout << "Read content: Got error: " << ec.message() << std::endl;
        request_->Fail(HttpStatusCode::bad_request);
        return;
    }

    auto handler_it = handlers_->find( request_->path );
    if ( handler_it == handlers_->end() )
    {
        std::cout << "Server error: no handler for path: " << request_->path
            << std::endl;
        request_->Fail(HttpStatusCode::not_found);
        return;
    }

    std::istream response_stream(&request_->read_buffer_);

    std::string line;
    if( std::getline(response_stream, line) )
    {
        boost::trim_right(line);
        if ( ! line.empty() )
        {
            request_->post = line;

            std::vector<std::string> post_parts;
            boost::split(
                post_parts,
                line,
                boost::is_any_of("&"));

            for ( const std::string & part : post_parts )
            {
                std::vector<std::string> key_pair;
                boost::split(
                    key_pair,
                    part,
                    boost::is_any_of("="));

                boost::optional<std::string> key = mf::utils::UrlPostUnencode(
                    key_pair[0] );
                if ( ! key )
                {
                    std::cout << "Got invalid POST data." << std::endl;
                    request_->Fail(HttpStatusCode::bad_request);
                    return;
                }

                if (key_pair.size() > 1)
                {
                    boost::optional<std::string> value =
                        mf::utils::UrlPostUnencode( key_pair[1] );
                    if ( ! value )
                    {
                        std::cout << "Got invalid POST data." << std::endl;
                        request_->Fail(HttpStatusCode::bad_request);
                        return;
                    }

                    request_->post_parts[*key] = *value;
                }
                else
                {
                    request_->post_parts[*key] = "";
                }
            }
        }
    }

    // Execute the handler.
    (handler_it->second)(std::move(request_));
}

