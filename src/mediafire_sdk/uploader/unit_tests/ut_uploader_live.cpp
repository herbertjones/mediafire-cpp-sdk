/**
 * @file ut_uploader_live.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */

// Avoid excessive debug messages by including headers we don't want debug from
// before setting OUTPUT_DEBUG
#include "mediafire_sdk/api/session_maintainer.hpp"
#define OUTPUT_DEBUG

#include "mediafire_sdk/api/upload/check.hpp"

#include "mediafire_sdk/uploader/upload_manager.hpp"


#include "boost/asio.hpp"
#include "boost/asio/impl/src.hpp"  // Define once in program
#include "boost/asio/ssl.hpp"
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program

#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_int_distribution.hpp"




#define BOOST_TEST_MODULE UtUploaderLive
#include "boost/test/unit_test.hpp"



#if ! defined(TEST_USER_1_USERNAME) || ! defined(TEST_USER_1_PASSWORD) \
 || ! defined(TEST_USER_2_USERNAME) || ! defined(TEST_USER_2_PASSWORD)
# error "TEST_USER defines not set."
#endif

namespace {
const std::string username = TEST_USER_1_USERNAME;
const std::string password = TEST_USER_1_PASSWORD;
}  // namespace

BOOST_AUTO_TEST_CASE(BlankTest)
{
    boost::asio::io_service io_service;

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    mf::api::SessionMaintainer session_maintainer(http_config);
    session_maintainer.SetLoginCredentials(
        mf::api::credentials::Email{ username, password } );

    mf::uploader::UploadManager upload_manager(&session_maintainer);

    /** @todo hjones: Create a random file */

    /** @todo hjones: Request file to upload */

    /** @todo hjones: Delete file after uploaded. */
    /** @todo hjones: Stop io_service after delete complete. */

    io_service.run();

    /** @todo hjones: Delete the random file */

    /** @todo hjones: Validate in some way */
    //BOOST_CHECK_EQUAL( 1, 1 );
}

