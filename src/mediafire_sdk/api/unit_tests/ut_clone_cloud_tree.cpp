/**
 * @file api/unit_tests/ut_clone_cloud_tree.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/actions/clone_cloud_tree.hpp"
#include "mediafire_sdk/api/actions/get_folder_content.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE CloneCloudTree
#include "boost/test/unit_test.hpp"

#if !defined(TEST_USER_1_USERNAME) || !defined(TEST_USER_1_PASSWORD)
#error "TEST_USER defines not set."
#endif

namespace
{
const std::string username = TEST_USER_1_USERNAME;
const std::string password = TEST_USER_1_PASSWORD;
const std::string root_folderkey = "";

namespace globals
{
uint32_t file_count = 0;
uint32_t folder_count = 0;
}  // namespace globals

}  // end anonymous namespace

namespace api = mf::api;

BOOST_AUTO_TEST_CASE(UTGetContentFiles)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    auto coro = api::GetFolderContent::Create(
            &stm,
            [&io_service, this](api::ActionResult result,
                                api::GetFolderContent::Pointer action)
            {
                if (result == api::ActionResult::Failure)
                {
                    std::ostringstream ss;
                    ss << "Failed with error: "
                       << action->GetErrorCode().message();
                    BOOST_FAIL(ss.str());
                }
                else
                {
                    // Should not retrieve folders
                    BOOST_CHECK_EQUAL(0, action->folders.size());

                    std::cout << "File count: " << action->files.size()
                              << std::endl;

                    globals::file_count = action->files.size();
                }

                io_service.stop();
            },
            root_folderkey, api::GetFolderContentType::Files);

    io_service.run();
}

BOOST_AUTO_TEST_CASE(UTGetContentFolders)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    auto coro = api::GetFolderContent::Create(
            &stm,
            [&io_service, this](api::ActionResult result,
                                api::GetFolderContent::Pointer action)
            {
                if (result == api::ActionResult::Failure)
                {
                    std::ostringstream ss;
                    ss << "Failed with error: "
                       << action->GetErrorCode().message();
                    BOOST_FAIL(ss.str());
                }
                else
                {
                    // Should not retrieve files
                    BOOST_CHECK_EQUAL(0, action->files.size());

                    std::cout << "Folder count: " << action->folders.size()
                              << std::endl;

                    globals::folder_count = action->folders.size();
                }

                io_service.stop();
            },
            root_folderkey, api::GetFolderContentType::Folders);

    io_service.run();
}

BOOST_AUTO_TEST_CASE(UTGetContentFilesAndFolders)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    auto coro = api::GetFolderContent::Create(
            &stm,
            [&io_service, this](api::ActionResult result,
                                api::GetFolderContent::Pointer action)
            {
                if (result == api::ActionResult::Failure)
                {
                    std::ostringstream ss;
                    ss << "Failed with error: "
                       << action->GetErrorCode().message();
                    BOOST_FAIL(ss.str());
                }
                else
                {
                    // Should not retrieve files
                    BOOST_CHECK_EQUAL(globals::file_count,
                                      action->files.size());
                    BOOST_CHECK_EQUAL(globals::folder_count,
                                      action->folders.size());

                    std::cout << "File count: " << action->files.size()
                              << std::endl;
                    std::cout << "Folder count: " << action->folders.size()
                              << std::endl;
                }

                io_service.stop();
            },
            root_folderkey, api::GetFolderContentType::FilesAndFolders);

    io_service.run();
}

BOOST_AUTO_TEST_CASE(UTCloneCloudTree)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    auto coro = api::CloneCloudTree::Create(
            &stm,
            [&io_service](api::ActionResult result,
                          api::CloneCloudTree::Pointer action)
            {
                if (result == api::ActionResult::Failure)
                {
                    std::cout << "Failed. " <<
                    action->GetErrorCode().message()
                              << ": " << action->GetErrorDescription()
                              << std::endl;
                }
                else
                {
                    std::cout << "CloneCloudTree done." << std::endl;
                }

                io_service.stop();
            },
            root_folderkey);

    io_service.run();
}
