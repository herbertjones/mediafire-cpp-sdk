/**
 * @file ut_hasher.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */

#define BOOST_TEST_MODULE UtHasher
#include "boost/test/unit_test.hpp"

#include "mediafire_sdk/uploader/detail/stepping.hpp"

BOOST_AUTO_TEST_CASE(MinimumSizes)
{
    using namespace mf::uploader::detail;

    BOOST_CHECK_EQUAL( SteppingMinFileSize(0), 0 );
    BOOST_CHECK_EQUAL( SteppingMinFileSize(1), MB(4) );
    BOOST_CHECK_EQUAL( SteppingMinFileSize(2), MB(16) );
    BOOST_CHECK_EQUAL( SteppingMinFileSize(3), MB(64) );
    BOOST_CHECK_EQUAL( SteppingMinFileSize(4), MB(256) );
    BOOST_CHECK_EQUAL( SteppingMinFileSize(5), GB(1) );
    BOOST_CHECK_EQUAL( SteppingMinFileSize(6), GB(4) );
    BOOST_CHECK_EQUAL( SteppingMinFileSize(7), GB(16) );
}

BOOST_AUTO_TEST_CASE(CalculatedStepping)
{
    using namespace mf::uploader::detail;

    BOOST_CHECK_EQUAL( 0, ThresholdStepping(0) );
    BOOST_CHECK_EQUAL( 0, ThresholdStepping(1) );
    BOOST_CHECK_EQUAL( 0, ThresholdStepping(MB(4)-1) );

    BOOST_CHECK_EQUAL( 1, ThresholdStepping(MB(4)) );
    BOOST_CHECK_EQUAL( 1, ThresholdStepping(MB(5)) );
    BOOST_CHECK_EQUAL( 1, ThresholdStepping(MB(10)) );
    BOOST_CHECK_EQUAL( 1, ThresholdStepping(MB(16)-1) );

    BOOST_CHECK_EQUAL( 2, ThresholdStepping(MB(16)) );
    BOOST_CHECK_EQUAL( 2, ThresholdStepping(MB(40)) );
    BOOST_CHECK_EQUAL( 2, ThresholdStepping(MB(64)-1) );

    BOOST_CHECK_EQUAL( 3, ThresholdStepping(MB(64)) );
    BOOST_CHECK_EQUAL( 3, ThresholdStepping(MB(256)-1) );

    BOOST_CHECK_EQUAL( 4, ThresholdStepping(MB(256)) );
    BOOST_CHECK_EQUAL( 4, ThresholdStepping(GB(1)-1) );

    BOOST_CHECK_EQUAL( 5, ThresholdStepping(GB(1)) );
    BOOST_CHECK_EQUAL( 5, ThresholdStepping(GB(4)-1) );

    BOOST_CHECK_EQUAL( 6, ThresholdStepping(GB(4)) );
    BOOST_CHECK_EQUAL( 6, ThresholdStepping(GB(16)-1) );

    BOOST_CHECK_EQUAL( 7, ThresholdStepping(GB(16)) );
    BOOST_CHECK_EQUAL( 7, ThresholdStepping(TB(1)) );
    BOOST_CHECK_EQUAL( 7, ThresholdStepping(TB(2)) );
}
