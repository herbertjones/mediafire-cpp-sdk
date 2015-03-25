/**
 * @file ut_variant.cpp
 * @author Herbert Jones
 * @brief Unit test for ../variant.cpp functions.
 *
 * @copyright Copyright 2015 Mediafire
 */

#include <string>
#include <iostream>

#include "boost/variant.hpp"
#include "mediafire_sdk/utils/variant.hpp"

#define BOOST_TEST_MODULE UtilsVariant
#include "boost/test/unit_test.hpp"

using VariantOf2 = boost::variant<int, std::string>;
using VariantOf3 = boost::variant<int, double, std::string>;
using mf::utils::Match;
using mf::utils::MatchPartial;
using mf::utils::MatchPartialWithDefault;

BOOST_AUTO_TEST_CASE(test_1)
{
    VariantOf3 v("test");

    Match(v,
          [](const int &)
          {
              BOOST_FAIL("Picked wrong type. (int)");
          },
          [](const double &)
          {
              BOOST_FAIL("Picked wrong type. (double)");
          },
          [](const std::string & val)
          {
              BOOST_CHECK_EQUAL(val, "test");
          });
}

BOOST_AUTO_TEST_CASE(test_2)
{
    VariantOf3 v(1);

    Match(v,
          [](const int & val)
          {
              BOOST_CHECK_EQUAL(val, 1);
          },
          [](const double &)
          {
              BOOST_FAIL("Picked wrong type. (double)");
          },
          [](const std::string &)
          {
              BOOST_FAIL("Picked wrong type. (string)");
          });
}

BOOST_AUTO_TEST_CASE(test_3)
{
    VariantOf3 v(1.0);

    Match(v,
          [](const int &)
          {
              BOOST_FAIL("Picked wrong type. (int)");
          },
          [](const double & val)
          {
              BOOST_CHECK_EQUAL(val, 1.0);
          },
          [](const std::string &)
          {
              BOOST_FAIL("Picked wrong type. (string)");
          });
}

BOOST_AUTO_TEST_CASE(test_4)
{
    VariantOf2 v(1);

    // This works as int can cast to double.
    int ret = Match(v,
                    [](const double &)
                    {
                        return 1;
                    },
                    [](const std::string &)
                    {
                        return 3;
                    });

    BOOST_CHECK_EQUAL(ret, 1);
}

BOOST_AUTO_TEST_CASE(partial_1)
{
    VariantOf2 v(0);

    int ret = 0;

    MatchPartial(v, [&](const std::string &)
                 {
                     ret = 1;
                 });

    BOOST_CHECK_EQUAL(ret, 0);
}

BOOST_AUTO_TEST_CASE(partial_2)
{
    VariantOf2 v(0);

    int ret = 0;

    MatchPartial(v, [&](const int &)
                 {
                     ret = 1;
                 });

    BOOST_CHECK_EQUAL(ret, 1);
}

BOOST_AUTO_TEST_CASE(partial_default_1)
{
    VariantOf2 v(0);

    // This works as int can cast to double.
    int ret = MatchPartialWithDefault(v, 1, [](const std::string &)
                                      {
                                          return 3;
                                      });

    BOOST_CHECK_EQUAL(ret, 1);
}

BOOST_AUTO_TEST_CASE(partial_default_2)
{
    VariantOf2 v(3.0);

    // This works as int can cast to double.
    int ret = MatchPartialWithDefault(v, 1, [](const std::string &)
                                      {
                                          return 2;
                                      });

    BOOST_CHECK_EQUAL(ret, 1);
}
