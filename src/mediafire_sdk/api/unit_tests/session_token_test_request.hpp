/**
 * @file session_token_test_request.hpp
 * @author Herbert Jones
 * @brief Request
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"

#include "session_token_test.hpp"

namespace mf {
namespace api {
namespace ut {

class Request
    : public std::enable_shared_from_this<Request>
{
public:
    Request(
            boost::asio::io_service * io_service_,
            boost::asio::ssl::context * ssl_ctx_,
            boost::asio::ip::tcp::socket socket_
        );
    virtual ~Request();

    boost::asio::io_service * io_service;

    /// Socket for the connection.
    boost::asio::ip::tcp::socket socket;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket&> ssl_stream;

    boost::asio::streambuf read_buffer_;

    // Like GET, POST
    std::string http_method;

    // Like /api/user/get_session_token.php?email=email&password=password
    std::string http_request_path;

    // Like HTTP/1.1
    std::string http_version;

    // Like /api/user/get_session_token.php
    std::string path;

    // Like email=email&password=password
    std::string query;

    std::map< std::string, std::string > query_parts;

    std::string post;

    std::map< std::string, std::string > post_parts;

    /**
     * HTTP header names are case insensitive, so the key value here is
     * converted to lowercase for ease of use.
     */
    std::map<std::string, std::string> headers;

    /** @todo hjones: implement post_data when needed */
    std::vector<uint8_t> post_data;

    void Fail(HttpStatusCode);

    void Respond(
            HttpStatusCode,
            const std::string & response
        );
};

}  // namespace ut
}  // namespace api
}  // namespace mf
