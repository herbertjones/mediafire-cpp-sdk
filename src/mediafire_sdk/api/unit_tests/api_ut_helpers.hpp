/**
 * @file api_ut_helpers.hpp
 * @author Herbert Jones
 * @brief Helpers for API unit tests.
 *
 * @copyright Copyright 2014 Mediafire
 *
 * Detailed message...
 */
#pragma once

#include <memory>
#include <string>
#include <system_error>

#include "mediafire_sdk/api/requester.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/http/http_config.hpp"
#include "mediafire_sdk/http/unit_tests/expect_server.hpp"
#include "mediafire_sdk/http/unit_tests/expect_server_ssl.hpp"

#include "boost/asio.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

namespace mf {
namespace api {
namespace ut {

static const std::string kLocalHost = "127.0.0.1";
static const uint16_t kPort = 49995;

boost::property_tree::wptree MakeBaseApiContent(
        std::string action
    );

boost::property_tree::wptree MakeErrorApiContent(
        std::string action,
        std::string api_error_text,
        int api_error_code
    );

std::string ToString(
        const boost::property_tree::wptree & pt
    );

template<typename ApiFunctor>
class ApiExpectServer
{
public:
    typedef typename ApiFunctor::ResponseType ApiFunctorDataType;

    ApiExpectServer(
            const boost::property_tree::wptree & pt,
            const std::string & request_header_regex,
            uint32_t response_http_code,
            std::string response_http_message
        ) :
        pt_(pt),
        request_header_regex_(request_header_regex),
        response_http_code_(response_http_code),
        response_http_message_(response_http_message)
    {
        // Default error code in case Callback isn't called and api_data
        // replaced.
        api_data.error_code = make_error_code(std::errc::broken_pipe);
    }

    void Run( ApiFunctor functor, typename ApiFunctor::CallbackType cb )
    {
        boost::asio::io_service io_service;

        std::string server_response;
        {
            std::wostringstream ss;
            boost::property_tree::write_json( ss, pt_ );
            server_response = mf::utils::wide_to_bytes(ss.str());
        }

        std::string response_headers;
        {
            std::ostringstream ss;
            ss << "HTTP/1.1 " << response_http_code_ << " "
                << response_http_message_ << "\r\n";
            ss << "Date: Wed, 26 Mar 2014 12:47:29 GMT\r\n";
            ss << "Server: Apache\r\n";
            ss << "Cache-control: no-cache, must-revalidate\r\n";
            ss << "Content-Length: " << server_response.size() << "\r\n";
            ss << "Pragma: no-cache\r\n";
            ss << "Expires: 0\r\n";
            ss << "Connection: close\r\n";
            ss << "Content-Type: text/html; charset=UTF-8\r\n";
            ss << "\r\n";
            response_headers = ss.str();
        }

        std::shared_ptr<boost::asio::io_service::work> work(
                std::make_shared<boost::asio::io_service::work>( io_service ) );
        std::shared_ptr<ExpectServerSsl> server =
            ExpectServerSsl::Create(
                    &io_service,
                    work,
                    kPort
                );
        work.reset();  // Let the expect server stop

        server->Push( ExpectHandshake{} );

        ExpectRegex expect_regex{boost::regex("\r\n\r\n")};
        server->Push( expect_regex );  // Headers

        server->Push( expect_server_test::SendMessage{ response_headers });
        server->Push( expect_server_test::SendMessage{ server_response });

        std::string host =
            kLocalHost +  std::string(":") + mf::utils::to_string(kPort);
        auto http_config = mf::http::HttpConfig::Create();
        http_config->SetWorkIoService(&io_service);
        http_config->AllowSelfSignedCertificate();
        api::Requester api( http_config, host );

        api.Call( functor, cb, api::RequestStarted::Yes );

        io_service.run();
    }

    void Callback( const ApiFunctorDataType & data )
    {
        api_data = data;
    }

    const ApiFunctorDataType & Data() { return api_data; }

private:
    const boost::property_tree::wptree & pt_;

    const std::string & request_header_regex_;

    uint32_t response_http_code_;
    std::string response_http_message_;

    ApiFunctorDataType api_data;
};

}  // namespace ut
}  // namespace api
}  // namespace mf
