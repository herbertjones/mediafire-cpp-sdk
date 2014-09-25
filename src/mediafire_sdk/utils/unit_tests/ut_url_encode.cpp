/**
 * @file ut_url_encode.cpp
 * @author Herbert Jones
 * @brief Unit test for ../url_encode.cpp functions.
 *
 * @copyright Copyright 2014 Mediafire
 */

#include <string>
#include <iostream>

#include "mediafire_sdk/utils/url_encode.hpp"
#define BOOST_TEST_MODULE UrlEncode
#include "boost/test/unit_test.hpp"

char const * const pairs[][2] = {
    // unencoded, encoded
    { "qwerty=/;", "qwerty%3D%2F%3B" },
    { " %", "%20%25" },
    { "hello_test", "hello_test" },
};

BOOST_AUTO_TEST_CASE(UrlEncode)
{
    for ( std::size_t i = 0; i < sizeof(pairs)/sizeof(pairs[0]); ++i )
    {
        BOOST_CHECK_EQUAL(
                mf::utils::url::get_parameter::Encode( pairs[i][0] ),
                std::string( pairs[i][1] )
            );
    }
}

BOOST_AUTO_TEST_CASE(UrlUnencode)
{
    for ( std::size_t i = 0; i < sizeof(pairs)/sizeof(pairs[0]); ++i )
    {
        boost::optional<std::string> maybe_decoded =
            mf::utils::url::get_parameter::Decode( pairs[i][1] );

        BOOST_CHECK(
                maybe_decoded
                && *maybe_decoded == std::string( pairs[i][0] )
            );
    }
}

