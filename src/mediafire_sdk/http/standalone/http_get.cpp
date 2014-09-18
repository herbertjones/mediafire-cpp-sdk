/**
 * @file main.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "boost/asio.hpp"
#include "boost/asio/impl/src.hpp"  // Define once in program
#include "boost/asio/ssl.hpp"
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#include "boost/bind.hpp"
#include "boost/program_options.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"

#include "mediafire_sdk/http/http_request.hpp"
#include "mediafire_sdk/http/request_response_interface.hpp"

namespace po = boost::program_options;

class ResponseReader : public mf::http::RequestResponseInterface
{
public:
    ResponseReader() :
        show_headers_(false)
    {}

    virtual void RedirectHeaderReceived(
            std::string raw_header,
            std::map<std::string, std::string> /* headers */,
            mf::http::Url new_url
        )
    {
        std::cout << "Redirect received: " << raw_header << std::endl;
        std::cout << "New URL: " << new_url.url() << std::endl;
    }

    virtual void ResponseHeaderReceived(
            mf::http::Headers headers
        )
    {
        if ( show_headers_ )
        {
            std::cout << headers.raw_headers << std::flush;
        }
    }

    virtual void ResponseContentReceived(
            std::size_t /* start_pos */,
            std::shared_ptr<mf::http::BufferInterface> buffer
        )
    {
        std::cout.write( reinterpret_cast<const char*>(buffer->Data()),
            buffer->Size() );
    }

    virtual void RequestResponseErrorEvent(
            std::error_code error_code,
            std::string error_text
        )
    {
        std::cerr << "Error(" << error_code.message() << "): "
            << error_text << std::endl;
    }

    virtual void RequestResponseCompleteEvent()
    {
    }

    void ShowHeaders()
    {
        show_headers_ = true;
    }

private:
    bool show_headers_;
};

// non 0 on failure
int SetProxy(
        mf::http::HttpConfig::Pointer http_config,
        const std::string & proxy ,
        const std::string & proxy_user,
        const std::string & proxy_password
    )
{
    if ( proxy.empty() )
        return 0;

    std::vector<std::string> parts;

    boost::split( parts, proxy, boost::is_any_of(":") );

    mf::http::Proxy http_proxy, https_proxy;

    if ( parts.size() == 1 )
    {
        // No port set.

        http_proxy.host = parts[0];
        https_proxy.host = parts[0];

        http_proxy.port = 80;
        https_proxy.port = 443;
    }
    else if ( parts.size() == 2 )
    {
        uint16_t port = 0;
        if ( sscanf( parts[1].c_str(), "%hi", &port ) != 1 )
            return 1;

        http_proxy.host = parts[0];
        https_proxy.host = parts[0];

        http_proxy.port = port;
        https_proxy.port = port;
    }
    else
    {
        return 1;
    }

    http_proxy.username = proxy_user;
    https_proxy.username = proxy_user;

    http_proxy.password = proxy_password;
    https_proxy.password = proxy_password;

    http_config->SetHttpProxy(http_proxy);
    http_config->SetHttpsProxy(https_proxy);

    return 0;
}

int main(int argc, char *argv[])
{
    try {
        std::string url;
        std::string proxy;
        std::string proxy_user;
        std::string proxy_password;
        std::string post_data;
        int bandwidth_usage_percentage = 100;

        po::options_description visible("Allowed options");
        visible.add_options()
            ("help,h", "Show this message.")
            ("headers,H", "Show received headers.")
            ("bandwidth_usage_percent,b",
                po::value<int>(&bandwidth_usage_percentage),
                "Set the bandwidth usage percent. Valid range: 1-100")
            ("post-data",
                po::value<std::string>(&post_data),
                "Post data to url.")
            ("proxy",
                po::value<std::string>(&proxy),
                "Set proxy. Has the form: \"host:port\"")
            ("proxyuser",
                po::value<std::string>(&proxy_user),
                "Set proxy authorization username.")
            ("proxypass",
                po::value<std::string>(&proxy_password),
                "Set proxy authorization username.");

        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("url", po::value<std::string>(&url), "url");


        po::positional_options_description p;
        p.add("url", 1);

        po::options_description cmdline_options;
        cmdline_options.add(visible).add(hidden);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
                options(cmdline_options).positional(p).run(), vm);
        po::notify(vm);

        auto http_config = mf::http::HttpConfig::Create();

        if (vm.count("help") || ! vm.count("url"))
        {
            std::cout << "Usage: " << argv[0] << " [options] URL\n";
            std::cout << visible << "\n";
            return 0;
        }

        if ( bandwidth_usage_percentage < 1
            || bandwidth_usage_percentage > 100 )
        {
            std::cout << "Invalid bandwidth usage percentage.\n";
            std::cout << "Usage: " << argv[0] << " [options] URL\n";
            std::cout << visible << "\n";
            return 0;
        }

        http_config->SetBandwidthUsagePercent(bandwidth_usage_percentage);

        std::shared_ptr<ResponseReader> rr(
                std::make_shared<ResponseReader>()
            );

        if ( vm.count("headers") )
        {
            rr->ShowHeaders();
        }

        if ( SetProxy( http_config, proxy, proxy_user, proxy_password ) != 0 )
        {
            std::cerr << "Error: Unable to parse proxy: " << proxy << "\n";
            return 1;
        }

        mf::http::HttpRequest::Pointer request(
            mf::http::HttpRequest::Create(
                http_config,
                std::static_pointer_cast<
                mf::http::RequestResponseInterface>(rr),
                url
            ));

        if ( ! post_data.empty() )
        {
            request->SetPostData(
                mf::http::SharedBuffer::Create(
                    std::move(post_data) ) );
        }

        // Start the request.
        request->Start();

        http_config->RunService();
    }
    catch(std::exception& e)
    {
        std::cerr << "Uncaught exception: " << e.what() << "\n";
        return 1;
    }
    catch(...)
    {
        std::cerr << "Exception of unknown type!\n";
        return 1;
    }

    return 0;
}
