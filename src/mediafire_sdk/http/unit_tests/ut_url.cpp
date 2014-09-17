/**
 * @file ut_url.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include <string>

#include "mediafire_sdk/http/url.hpp"
#define BOOST_TEST_MODULE UrlUnitTest
#include "boost/test/unit_test.hpp"

BOOST_AUTO_TEST_CASE(SslIpPort)
{
    std::string url = "https://127.0.0.1:49995/api/user/get_session_token.php"
        "?email=test%40test.com"
        "&password=hunter2"
        "&application_id=1"
        "&signature=39402adf8e09200c192516eb7740a25bc72791e2"
        "&token_version=2"
        "&response_format=json";

    mf::http::Url ssl_url(url);

    BOOST_CHECK_EQUAL( ssl_url.scheme(), "https" );
    BOOST_CHECK_EQUAL( ssl_url.port(), "49995" );
    BOOST_CHECK_EQUAL( ssl_url.query(),
            "email=test%40test.com"
            "&password=hunter2"
            "&application_id=1"
            "&signature=39402adf8e09200c192516eb7740a25bc72791e2"
            "&token_version=2"
            "&response_format=json"
        );
}

BOOST_AUTO_TEST_CASE( relative_url )
{
    std::string url = "//www.mediafire.com/index.html";

    BOOST_CHECK_THROW(
            mf::http::Url relative_url(url),
            mf::http::InvalidUrl
        );
}

BOOST_AUTO_TEST_CASE( fragment )
{
    std::string urlString = "http://www.mediafire.com/index.html/?#someplace";

    mf::http::Url url(urlString);
    BOOST_CHECK_EQUAL( url.fragment(), "someplace" );
}
