/**
 * @file session_token_test_server.hpp
 * @author Herbert Jones
 * @brief Unit test server for session token APIs
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"

#include "session_token_test_request.hpp"

namespace mf {
namespace api {
namespace ut {

class SessionTokenTestServer
{
public:
    SessionTokenTestServer(
            boost::asio::io_service * io_service,
            const std::string& address,
            const std::string& port,
            PathHandlers handlers
        );
    ~SessionTokenTestServer();

private:
    /// The io_service used to perform asynchronous operations.
    boost::asio::io_service * io_service_;

    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor acceptor_;

    /// The next socket to be accepted.
    boost::asio::ip::tcp::socket socket_;

    boost::asio::ssl::context ssl_ctx_;

    PathHandlers handlers_;

    void DoAccept();
};

}  // namespace ut
}  // namespace api
}  // namespace mf
