/**
 * @file ut_container_algorithms.cpp
 * @author Herbert Jones
 * @brief Unit test for ../container_algorithms.hpp functions.
 *
 * @copyright Copyright 2015 Mediafire
 */

#include <algorithm>
#include <string>
#include <iostream>
#include <vector>

#include "mediafire_sdk/utils/container_algorithms.hpp"
#define BOOST_TEST_MODULE UrlEncode
#include "boost/test/unit_test.hpp"

template <typename T>
bool IsEqual(const T & a, const T & b)
{
    if (a.size() == b.size())
    {
        return std::equal(std::begin(a), std::end(a), std::begin(b));
    }
    else
    {
        return false;
    }
}

BOOST_AUTO_TEST_CASE(Test_1)
{
    std::vector<int> a = {1, 2, 3, 4, 5};
    std::vector<int> b = {1, 2, 3, 4, 5};
    std::vector<int> c;

    std::vector<int> expected_a = {};
    std::vector<int> expected_b = {};
    std::vector<int> expected_c = {1, 2, 3, 4, 5};

    mf::utils::RepartitionIntersection(a, b, c);
    BOOST_CHECK(IsEqual(a, expected_a));
    BOOST_CHECK(IsEqual(b, expected_b));
    BOOST_CHECK(IsEqual(c, expected_c));
}

BOOST_AUTO_TEST_CASE(Test_2)
{
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {3, 4, 5};
    std::vector<int> c;

    std::vector<int> expected_a = {1, 2};
    std::vector<int> expected_b = {4, 5};
    std::vector<int> expected_c = {3};

    mf::utils::RepartitionIntersection(a, b, c);
    BOOST_CHECK(IsEqual(a, expected_a));
    BOOST_CHECK(IsEqual(b, expected_b));
    BOOST_CHECK(IsEqual(c, expected_c));
}

BOOST_AUTO_TEST_CASE(Test_3)
{
    std::vector<int> a = {1, 2, 3, 4, 5};
    std::vector<int> b = {3, 4, 5};
    std::vector<int> c;

    std::vector<int> expected_a = {1, 2};
    std::vector<int> expected_b = {};
    std::vector<int> expected_c = {3, 4, 5};

    mf::utils::RepartitionIntersection(a, b, c);
    BOOST_CHECK(IsEqual(a, expected_a));
    BOOST_CHECK(IsEqual(b, expected_b));
    BOOST_CHECK(IsEqual(c, expected_c));
}

BOOST_AUTO_TEST_CASE(Test_4)
{
    std::vector<int> a = {1, 2};
    std::vector<int> b = {1, 2, 3, 4, 5};
    std::vector<int> c;

    std::vector<int> expected_a = {};
    std::vector<int> expected_b = {3, 4, 5};
    std::vector<int> expected_c = {1, 2};

    mf::utils::RepartitionIntersection(a, b, c);
    BOOST_CHECK(IsEqual(a, expected_a));
    BOOST_CHECK(IsEqual(b, expected_b));
    BOOST_CHECK(IsEqual(c, expected_c));
}
