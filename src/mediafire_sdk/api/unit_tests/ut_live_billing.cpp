/**
 * @file api/unit_tests/ut_live_billing.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "ut_live.hpp"

#ifndef OUTPUT_DEBUG
#  define OUTPUT_DEBUG
#endif

#include "mediafire_sdk/api/billing/get_products.hpp"
#include "mediafire_sdk/api/billing/get_plans.hpp"

#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE ApiLiveBilling
#include "boost/test/unit_test.hpp"

namespace api = mf::api;

BOOST_FIXTURE_TEST_SUITE( s, ut::Fixture )

BOOST_AUTO_TEST_CASE(GetBillingProducts)
{
    api::billing::get_products::Request request;
    request.SetActive(api::billing::get_products::Activity::Active);

    Call(
        request,
        [&](const api::billing::get_products::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                BOOST_CHECK( ! response.products.empty() );

                for ( const api::billing::get_products::Response::Product & it
                    : response.products )
                {
                    BOOST_CHECK( ! it.description.empty() );
                }
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(GetBillingPlans)
{
    Call(
        api::billing::get_plans::Request(),
        [&](const api::billing::get_plans::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                BOOST_CHECK( ! response.plans.empty() );

                for ( const api::billing::get_plans::Response::Plan & it
                    : response.plans )
                {
                    BOOST_CHECK( ! it.description.empty() );
                }
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_SUITE_END()
