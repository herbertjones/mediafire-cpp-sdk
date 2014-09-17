/**
 * @file ut_sha1_hasher.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include <iostream>
#include <string>

#include "../sha1_hasher.hpp"
#define BOOST_TEST_MODULE Sha1Hasher
#include "boost/test/unit_test.hpp"

char const * const test_str = "This is a test.\n";

BOOST_AUTO_TEST_CASE(HashSha1)
{
    BOOST_CHECK_EQUAL(
            mf::utils::HashSha1(test_str),
            std::string("0828324174b10cc867b7255a84a8155cf89e1b8b") );
}

BOOST_AUTO_TEST_CASE(Sha1_1)
{
    mf::utils::Sha1Hasher hasher;
    hasher.Update(strlen(test_str), test_str);
    std::string digest = hasher.Digest();
    std::cout << "Got digest: " << digest << std::endl;
    BOOST_CHECK( digest == "0828324174b10cc867b7255a84a8155cf89e1b8b" );
}

BOOST_AUTO_TEST_CASE(Sha1_1000)
{
    mf::utils::Sha1Hasher hasher;
    for (int i = 0; i < 1000; ++i)
        hasher.Update(strlen(test_str), test_str);
    std::string digest = hasher.Digest();
    std::cout << "Got digest: " << digest << std::endl;
    BOOST_CHECK( digest == "62e17a6929b568cff57a031586f09619eac271cf" );
}

BOOST_AUTO_TEST_CASE(Sha1_1000000)
{
    mf::utils::Sha1Hasher hasher;
    for (int i = 0; i < 1000000; ++i)
        hasher.Update(strlen(test_str), test_str);
    std::string digest = hasher.Digest();
    std::cout << "Got digest: " << digest << std::endl;
    BOOST_CHECK( digest == "e5dff16982b332c1f78d59e37db1405674d51674" );
}

