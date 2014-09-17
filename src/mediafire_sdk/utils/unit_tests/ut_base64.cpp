/**
 * @file ut_base64.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include <string>
#include <vector>

#include "mediafire_sdk/utils/base64.hpp"
#define BOOST_TEST_MODULE Base64
#include "boost/test/unit_test.hpp"

char const * const pairs[][2] = {
    // unencoded, encoded
    { "", "" },
    { "Z", "Wg==" },
    { "a test string~~", "YSB0ZXN0IHN0cmluZ35+" },
    { "a test string~",  "YSB0ZXN0IHN0cmluZ34=" },
    { "a test string",   "YSB0ZXN0IHN0cmluZw==" },
    { "a test strin",    "YSB0ZXN0IHN0cmlu" },
    { "a test stri",     "YSB0ZXN0IHN0cmk=" },
    { "a test str",      "YSB0ZXN0IHN0cg==" },
};

BOOST_AUTO_TEST_CASE(Base64Encode)
{
    for ( std::size_t i = 0; i < sizeof(pairs)/sizeof(pairs[0]); ++i )
    {
        BOOST_CHECK_EQUAL(
                mf::utils::Base64Encode( pairs[i][0], strlen(pairs[i][0]) ),
                std::string( pairs[i][1] )
            );
    }
}

BOOST_AUTO_TEST_CASE(Base64Decode)
{
    for ( std::size_t i = 0; i < sizeof(pairs)/sizeof(pairs[0]); ++i )
    {
        boost::optional<std::vector<uint8_t>> maybe_decoded =
            mf::utils::Base64Decode( pairs[i][1] );

        std::string as_string;
        if ( maybe_decoded )
        {
            as_string = std::string(
                    maybe_decoded->begin(), maybe_decoded->end());
        }

        BOOST_CHECK(
                maybe_decoded
                && as_string == std::string( pairs[i][0] )
            );
    }
}

