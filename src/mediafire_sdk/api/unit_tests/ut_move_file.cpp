#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/actions/move_file.hpp"

#include "mediafire_sdk/api/file/move.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE MoveFile
#include "boost/test/unit_test.hpp"

#if !defined(TEST_USER_1_USERNAME) || !defined(TEST_USER_1_PASSWORD)
#error "TEST_USER defines not set."
#endif

namespace
{
const std::string username = TEST_USER_1_USERNAME;
const std::string password = TEST_USER_1_PASSWORD;
}  // end anonymous namespace

namespace api = mf::api;

BOOST_AUTO_TEST_CASE(UTMoveFile)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    using MoveFileType = mf::api::MoveFile<mf::api::file::move::Request>;

    using RequestType = MoveFileType::RequestType;
    using ResponseType = MoveFileType::ResponseType;

    MoveFileType::CallbackType HandleMoveFile = [this, &io_service](
            const ResponseType & response,
            const std::vector<MoveFileType::ErrorType> & errors)
    {
        if (!errors.empty())
        {
            BOOST_FAIL("HandleMoveFile reported errors!");
        }
        else
        {

        }

        io_service.stop();
    };

    auto move_file = MoveFileType::Create(
            &stm,
            "",
            "",
            std::move(HandleMoveFile));

    // No way to actually test this since it will modify an account, we just
    // need to make sure everything compile.

    //get_folder_info->operator()();

    //io_service.run();
}