#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/actions/delete_folder.hpp"

#include "mediafire_sdk/api/folder/folder_delete.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE DeleteFolder
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

BOOST_AUTO_TEST_CASE(UTDeleteFolder)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    using DeleteFolderType
            = mf::api::DeleteFolder<mf::api::folder::folder_delete::Request>;

    using RequestType = DeleteFolderType::RequestType;
    using ResponseType = DeleteFolderType::ResponseType;

    DeleteFolderType::CallbackType HandleDeleteFolder = [this, &io_service](
            const ResponseType & response,
            const std::vector<DeleteFolderType::ErrorType> & errors)
    {
        if (!errors.empty())
        {
            BOOST_FAIL("HandleDeleteFolder reported errors!");
        }
        else
        {
        }

        io_service.stop();
    };

    auto delete_folder
            = DeleteFolderType::Create(&stm, "", std::move(HandleDeleteFolder));

    // No way to actually test this since it will modify an account, we just
    // need to make sure everything compile.

    // get_folder_info->operator()();

    // io_service.run();
}