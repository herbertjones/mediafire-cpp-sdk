/**
 * @file session_token_test_connection.hpp
 * @author Herbert Jones
 * @brief Manage a server connection
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <memory>

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"

#include "session_token_test_request.hpp"

namespace mf {
namespace api {
namespace ut {

class SessionTokenTestConnection
    : public std::enable_shared_from_this<SessionTokenTestConnection>
{
public:
    SessionTokenTestConnection(const SessionTokenTestConnection&) = delete;
    SessionTokenTestConnection& operator=(const SessionTokenTestConnection&)
        = delete;

    /// Construct a connection with the given socket.
    explicit SessionTokenTestConnection(
            boost::asio::io_service * io_service,
            boost::asio::ssl::context * ssl_ctx,
            boost::asio::ip::tcp::socket socket,
            PathHandlers * handlers
        );
    virtual ~SessionTokenTestConnection() {}

    void Start();

private:
    std::shared_ptr<Request> request_;

    PathHandlers * handlers_;

    void Handshake();
    void ReadHeaders();
    void ParseHeaders(
            const std::size_t bytes_transferred
        );
    void HandleContentRead(
            const std::size_t bytes_transferred,
            const boost::system::error_code& err
        );
};

typedef std::shared_ptr<SessionTokenTestConnection> Connection;

}  // namespace ut
}  // namespace api
}  // namespace mf
