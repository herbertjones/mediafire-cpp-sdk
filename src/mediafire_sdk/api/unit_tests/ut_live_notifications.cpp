/**
 * @file api/unit_tests/ut_live_notifications.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "ut_live.hpp"

#ifndef OUTPUT_DEBUG
#  define OUTPUT_DEBUG
#endif

#include "mediafire_sdk/api/notifications/get_cache.hpp"
#include "mediafire_sdk/api/notifications/peek_cache.hpp"

#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE ApiLiveNotifications
#include "boost/test/unit_test.hpp"

namespace api = mf::api;

BOOST_FIXTURE_TEST_SUITE( s, ut::Fixture )

BOOST_AUTO_TEST_CASE(NotificationPeekCache)
{
    Call(
        api::notifications::peek_cache::Request(),
        [&](const api::notifications::peek_cache::Response & response)
        {
            if ( response.error_code )
                Fail(response);
            else
                Success();
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(NotificationGetCache)
{
    Call(
        api::notifications::get_cache::Request(),
        [&](const api::notifications::get_cache::Response & response)
        {
            if ( response.error_code )
                Fail(response);
            else
                Success();
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_SUITE_END()
