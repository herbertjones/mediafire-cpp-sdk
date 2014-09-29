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

/**
 * Self signed cert
 *
 * To generate another:
 * openssl genrsa -des3 -out server.key 1024
 * openssl req -new -key server.key -out server.csr
 * cp server.key server.key.org
 * openssl rsa -in server.key.org -out server.key
 * openssl x509 -req -days 3650 -in server.csr -signkey server.key -out server.crt
 */
char const  * const kPemCertificate =
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIICXQIBAAKBgQDicODW3qiEH77ZxJPz+QhB7Y2tgu/TkSaoyksO45wyo5zkzXO+\n"
    "TFSqC6AwJ/AzB8HMa0ZiYbmP0NUGbCrERWTBdzo5WjkPFmrETOQLUWlemuQtryaR\n"
    "IlFeket/vyWCLvwts3vNzlF4Hyxnv1ffifgubw4BKtc0OI13HTTf8J4iGQIDAQAB\n"
    "AoGAJFoMrlH2aaeTAvC888jB25ugR/+iMqu1shkvrYc6tyJu+IhHEYC9gsL1B2YR\n"
    "+I9BPGjoVrjrijvzRuGsh++/+cW1QQyrFge3jRVV2t0kESQ2NCfqsgdskjsjO9XS\n"
    "5YHTpThUCyV2mBDnh7LolyLOlp24nD3Yi3ugu+vlbDqaxsUCQQDw1pF4cJBicG3K\n"
    "q55mkuNo01eZPApalcHMjgMV9PZOUF7JNvo6qhtD2qV4cv67QJ5e/+S9qvdKHvx9\n"
    "W0JNfn4DAkEA8LJHgO/4ID4PPf6CgAuLB3OXkrIZ6iYAZPlDTuxiNAFoehD2ezTE\n"
    "47MSyCPflMnf6evBN8r/iWxGli6ITBACswJAEAl5rldwhd2OsgwzhAkL85L/JkkF\n"
    "N3r5aLGcKv4g2J4pcaSjjPx+zEnm8tpVdAqdgR3xEWAtD1Z44bAN/jMKGQJBAIrQ\n"
    "9yYov/ywbg/+CfuZLKy2gNNs/j8pfY6+p5AMCrMdoMjNoan7DBaaf5mH/vmL2CTM\n"
    "ABqSbAAwvyD8Y0Ui8rsCQQDlUvoW8fa3Q3wQ88FaBPHBhD0qsDWLHfBPLUGTmkhv\n"
    "4ZovBFmthiM3NP7VzpdIYVtTdEO6MmHclyNIszeXYgDG\n"
    "-----END RSA PRIVATE KEY-----\n"
    "-----BEGIN CERTIFICATE-----\n"
    "MIICSTCCAbICCQCxgYWdtbXq4TANBgkqhkiG9w0BAQUFADBpMQswCQYDVQQGEwJV\n"
    "UzEOMAwGA1UECAwFVGV4YXMxEDAOBgNVBAcMB0hvdXN0b24xEjAQBgNVBAoMCU1l\n"
    "ZGlhRmlyZTEQMA4GA1UECwwHRGVza3RvcDESMBAGA1UEAwwJbWVkaWFmaXJlMB4X\n"
    "DTE0MDkyOTE0MTExNFoXDTI0MDkyNjE0MTExNFowaTELMAkGA1UEBhMCVVMxDjAM\n"
    "BgNVBAgMBVRleGFzMRAwDgYDVQQHDAdIb3VzdG9uMRIwEAYDVQQKDAlNZWRpYUZp\n"
    "cmUxEDAOBgNVBAsMB0Rlc2t0b3AxEjAQBgNVBAMMCW1lZGlhZmlyZTCBnzANBgkq\n"
    "hkiG9w0BAQEFAAOBjQAwgYkCgYEA4nDg1t6ohB++2cST8/kIQe2NrYLv05EmqMpL\n"
    "DuOcMqOc5M1zvkxUqgugMCfwMwfBzGtGYmG5j9DVBmwqxEVkwXc6OVo5DxZqxEzk\n"
    "C1FpXprkLa8mkSJRXpHrf78lgi78LbN7zc5ReB8sZ79X34n4Lm8OASrXNDiNdx00\n"
    "3/CeIhkCAwEAATANBgkqhkiG9w0BAQUFAAOBgQC+gV8XlQyGHLdimckTmvGxgqqg\n"
    "IQrX63dDXDrertvZ9SkcT7Ae0Rx3+IvzpGc7rCq9AaBXqEf4p5laxsOz+gCXOlFG\n"
    "8D60iIqCkv+3aIPNobnAczRvlqo7wFBxQ/11BQ1BTWBpx4grg4sRq7twLBxUHL9y\n"
    "uXpH76ov1Z96Zw3cuQ==\n"
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

