/**
 * @file session_token_test_server.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "session_token_test_server.hpp"

#include <iostream>
#include <string>

#include "session_token_test_connection.hpp"

namespace au = mf::api::ut;
namespace asio = boost::asio;

namespace {
    char const  * const kPemCertificate =
        "-----BEGIN PRIVATE KEY-----\n"
        "MIICdgIBADANBgkqhkiG9w0BAQEFAASCAmAwggJcAgEAAoGBAMi5assnE7RG2iYF\n"
        "NIOqA7lbVAH3g/NN3bflghKFZCUtKe3qlS3dgDujs4EvPA1yGrV4EUOs0XvFYebC\n"
        "x+1BbsWAmQZnHQSHztAt6+DTCywJQJJUlbKwqsaanrnJ/dvARTEckXYKLRcgmfjf\n"
        "fYqXyDNQxiV6pPa2KsVMwoGBUxLNAgMBAAECgYAVajLKmdCwWx7LD6MaqPjcvbGo\n"
        "xA9/1b7h78qAz1pv3PGsQGrKCee0dTKhhbiSdqoC0lyFK9rtqZFYXU+XMHPwzfdx\n"
        "IIOq/lagyzgEqTvHA0GEK1lzO8Eegsf5y67FWZXNlRpKmFPCmFFyEpOUKHK0x6HN\n"
        "61/iZjMTXRo/AvFcAQJBAOY7BPjC7rphMqcLtw6Bb+WRId90VF72h3Q/LqLxtT2E\n"
        "Vv+n/+Ecl0C9UjhAQ/+wCHCkE/M5/VmS80fKzNUXDVECQQDfMO5kzaeYpUyd/Elb\n"
        "E2RNjTC4AAdtqel6fiGShfYrDxajXEq25CnnVFyxk7q6/ehM2mVnR1IuswDgvBhd\n"
        "It69AkEAkCiseEc2zCVIXiiLut15fzldCFoC6mNbdYKKZSUL4zUWdIZxRjdszfC9\n"
        "ptM2wMcswbs7crUA2jGVe4KUt2jzwQJARUT2eCqrvWBwKwhF7BJUqw0K9dBsfcii\n"
        "QfYrjUIuaKbCK+lU9vZRWw5/xk1HQwnSsyeFGUy1YPEFcLpwBVfxMQJAEj4TJ4rI\n"
        "T7lwtPOSek4nsVawxR95aAaqL1Qt+Khti9qAP6WM3OU7coRnfLjYLyPjpHnbFgZP\n"
        "7vv+EceHXehx8w==\n"
        "-----END PRIVATE KEY-----\n"
        "-----BEGIN CERTIFICATE-----\n"
        "MIIC7jCCAlegAwIBAgIJAPSjVyLAs5aJMA0GCSqGSIb3DQEBBQUAMIGPMQswCQYD\n"
        "VQQGEwJVUzEOMAwGA1UECAwFVGV4YXMxEDAOBgNVBAcMB0hvdXN0b24xEjAQBgNV\n"
        "BAoMCU1lZGlhRmlyZTEQMA4GA1UECwwHRGVza3RvcDESMBAGA1UEAwwJTWVkaWFG\n"
        "aXJlMSQwIgYJKoZIhvcNAQkBFhVoZXJiZXJ0QG1lZGlhZmlyZS5jb20wHhcNMTQw\n"
        "MzMxMjExNTUyWhcNMTQwNDMwMjExNTUyWjCBjzELMAkGA1UEBhMCVVMxDjAMBgNV\n"
        "BAgMBVRleGFzMRAwDgYDVQQHDAdIb3VzdG9uMRIwEAYDVQQKDAlNZWRpYUZpcmUx\n"
        "EDAOBgNVBAsMB0Rlc2t0b3AxEjAQBgNVBAMMCU1lZGlhRmlyZTEkMCIGCSqGSIb3\n"
        "DQEJARYVaGVyYmVydEBtZWRpYWZpcmUuY29tMIGfMA0GCSqGSIb3DQEBAQUAA4GN\n"
        "ADCBiQKBgQDIuWrLJxO0RtomBTSDqgO5W1QB94PzTd235YIShWQlLSnt6pUt3YA7\n"
        "o7OBLzwNchq1eBFDrNF7xWHmwsftQW7FgJkGZx0Eh87QLevg0wssCUCSVJWysKrG\n"
        "mp65yf3bwEUxHJF2Ci0XIJn4332Kl8gzUMYleqT2tirFTMKBgVMSzQIDAQABo1Aw\n"
        "TjAdBgNVHQ4EFgQULKVRo3R7fYQgUMjH5P4WR4nXqFswHwYDVR0jBBgwFoAULKVR\n"
        "o3R7fYQgUMjH5P4WR4nXqFswDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQUFAAOB\n"
        "gQDGbN4rijD2U08t3CmGyQQwpq3jd2hHY5XggBr6ir9hHSZIFxq0zf4eCATRhKKQ\n"
        "sYq3HokxmzzjOS/7Cs+Y0w70URgStkYale5Cc0oAIAABJmSrR4jOH3WXyOFz7Zrm\n"
        "2mv1n5e34d6/1dsmWOFQaQgtKahMrabqLAtH4eVpBO//aQ==\n"
        "-----END CERTIFICATE-----\n";

}  // namespace

au::SessionTokenTestServer::SessionTokenTestServer(
        boost::asio::io_service * io_service,
        const std::string& address,
        const std::string& port,
        PathHandlers handlers
    ) :
    io_service_(io_service),
    acceptor_(*io_service_),
    socket_(*io_service_),
    ssl_ctx_(asio::ssl::context::sslv23_server),
    handlers_(std::move(handlers))
{
    ssl_ctx_.set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2
        );

    ssl_ctx_.use_certificate_chain(
            asio::const_buffer(kPemCertificate, strlen(kPemCertificate)) );

    ssl_ctx_.use_private_key(
            asio::const_buffer(kPemCertificate, strlen(kPemCertificate)),
            boost::asio::ssl::context::pem );

    boost::asio::ip::tcp::resolver resolver(*io_service_);
    boost::asio::ip::tcp::endpoint endpoint =
        *resolver.resolve({address, port});
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    DoAccept();
}

au::SessionTokenTestServer::~SessionTokenTestServer()
{
}

void au::SessionTokenTestServer::DoAccept()
{
    acceptor_.async_accept(
        socket_,
        [this](boost::system::error_code ec)
        {
            // Check whether the server was stopped by a signal before this
            // completion handler had a chance to run.
            if (!acceptor_.is_open())
            {
                return;
            }

            if (!ec)
            {
                Connection connection =
                    std::make_shared<SessionTokenTestConnection>(
                        io_service_, &ssl_ctx_, std::move(socket_), &handlers_);
                connection->Start();
            }

            DoAccept();
        });
}

