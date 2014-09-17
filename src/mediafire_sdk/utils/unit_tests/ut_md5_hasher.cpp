/**
 * @file ut_md5_hasher.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include <iostream>
#include <string>

#include "mediafire_sdk/utils/md5_hasher.hpp"
#define BOOST_TEST_MODULE Md5Hasher
#include "boost/test/unit_test.hpp"

char const * const test_str = "This is a test.\n";

BOOST_AUTO_TEST_CASE(HashMd5)
{
    BOOST_CHECK_EQUAL(
            mf::utils::HashMd5(test_str),
            std::string("02bcabffffd16fe0fc250f08cad95e0c") );
}

BOOST_AUTO_TEST_CASE(Md5_1)
{
    mf::utils::Md5Hasher hasher;
    hasher.Update(strlen(test_str), test_str);
    std::string digest = hasher.Digest();
    std::cout << "Got digest: " << digest << std::endl;
    BOOST_CHECK( digest == "02bcabffffd16fe0fc250f08cad95e0c" );
}

BOOST_AUTO_TEST_CASE(Md5_1000)
{
    mf::utils::Md5Hasher hasher;
    for (int i = 0; i < 1000; ++i)
        hasher.Update(strlen(test_str), test_str);
    std::string digest = hasher.Digest();
    std::cout << "Got digest: " << digest << std::endl;
    BOOST_CHECK( digest == "4e28f7dc81db089321fcf6acd882e850" );
}

BOOST_AUTO_TEST_CASE(Md5_1000000)
{
    mf::utils::Md5Hasher hasher;
    for (int i = 0; i < 1000000; ++i)
        hasher.Update(strlen(test_str), test_str);
    std::string digest = hasher.Digest();
    std::cout << "Got digest: " << digest << std::endl;
    BOOST_CHECK( digest == "f0a6ee967cc8ab64b7fb0d28e7e49ff7" );
}

