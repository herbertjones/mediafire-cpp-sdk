/**
 * @file ut_sha256_hasher.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include <iostream>
#include <string>

#include "../sha256_hasher.hpp"
#define BOOST_TEST_MODULE Sha256Hasher
#include "boost/test/unit_test.hpp"

char const * const test_str = "This is a test.\n";

BOOST_AUTO_TEST_CASE(HashSha256)
{
    BOOST_CHECK_EQUAL(
            mf::utils::HashSha256(test_str),
            std::string("11586d2eb43b73e539caa3d158c883336c0e2c904b309c0c5ffe2c9b83d562a1") );
}

BOOST_AUTO_TEST_CASE(Sha256_1)
{
    mf::utils::Sha256Hasher hasher;
    hasher.Update(strlen(test_str), test_str);
    std::string digest = hasher.Digest();
    std::cout << "Got digest: " << digest << std::endl;
    BOOST_CHECK( digest == "11586d2eb43b73e539caa3d158c883336c0e2c904b309c0c5ffe2c9b83d562a1" );
}

BOOST_AUTO_TEST_CASE(Sha256_1000)
{
    mf::utils::Sha256Hasher hasher;
    for (int i = 0; i < 1000; ++i)
        hasher.Update(strlen(test_str), test_str);
    std::string digest = hasher.Digest();
    std::cout << "Got digest: " << digest << std::endl;
    BOOST_CHECK( digest == "00c5724217c519ca49adfced20357fe3a6b7f0719086aaf3a7690ae651674bde" );
}

BOOST_AUTO_TEST_CASE(Sha256_1000000)
{
    mf::utils::Sha256Hasher hasher;
    for (int i = 0; i < 1000000; ++i)
        hasher.Update(strlen(test_str), test_str);
    std::string digest = hasher.Digest();
    std::cout << "Got digest: " << digest << std::endl;
    BOOST_CHECK( digest == "b1e2ef542c8a634ba7d69ed9b5b6125561e172df74f78b177f0d2529522f5c3f" );
}

