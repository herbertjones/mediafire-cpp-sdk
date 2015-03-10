/**
 * @file api/unit_tests/ut_clone_cloud_tree.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/actions/clone_cloud_tree.hpp"
#include "mediafire_sdk/api/actions/get_folder_contents.hpp"

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

BOOST_AUTO_TEST_CASE(UTFolderGetContents2)
{
//    boost::asio::io_service io_service;
//    auto http_config = mf::http::HttpConfig::Create();
//    http_config->SetWorkIoService(&io_service);
//
//    api::SessionMaintainer stm(http_config);
//
//    stm.SetLoginCredentials(api::credentials::Email{username, password});
//
//    using ResponseType = mf::api::folder::get_content::Response;
//    using ResponseTypeFiles = std::vector<ResponseType::File>;
//    using ResponseTypeFolders = std::vector<ResponseType::Folder>;
//
//    mf::api::GetFolderContents::CallbackType HandleGetFolderContents =
//        [this, &io_service](const ResponseTypeFiles & response_files, const ResponseTypeFolders & response_folders)
//        {
//            for (const auto & file : response_files)
//            {
//                std::cout << "File: " << file.quickkey << std::endl;
//            }
//
//            for (const auto & folder : response_folders)
//            {
//                std::cout << "Folder: " << folder.folderkey << std::endl;
//            }
//
//            io_service.stop();
//        };
//
//    auto get_folder_contents = mf::api::GetFolderContents::Create(&stm, "", std::move(HandleGetFolderContents));
//    get_folder_contents->operator()();
//
//    io_service.run();
}

BOOST_AUTO_TEST_CASE(UTCloudCloneTree2)
{
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    mf::api::CloneCloudTree::CallbackType HandleCloneCloudTree =
    [this, &io_service](const std::vector<mf::api::CloneCloudTree::File> & response_files, const std::vector<mf::api::CloneCloudTree::Folder> & response_folders)
    {
        for (const auto & file : response_files)
        {
            std::cout << "File: " << file.quickkey << std::endl;
        }

        for (const auto & folder : response_folders)
        {
            std::cout << "Folder: " << folder.folderkey << std::endl;
        }

        std::cout << "File Count: " << response_files.size() << std::endl;
        std::cout << "Folder count: " << response_folders.size() << std::endl;

        io_service.stop();
    };

    auto clone_cloud_tree = mf::api::CloneCloudTree::Create(&stm, "", std::move(HandleCloneCloudTree));
    clone_cloud_tree->operator()();

    io_service.run();

    end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;

    std::cout << elapsed_seconds.count() << std::endl;
}

BOOST_AUTO_TEST_CASE(UTGetContentFiles)
{
//    boost::asio::io_service io_service;
//    auto http_config = mf::http::HttpConfig::Create();
//    http_config->SetWorkIoService(&io_service);
//
//    api::SessionMaintainer stm(http_config);
//
//    stm.SetLoginCredentials(api::credentials::Email{username, password});
//
//    auto coro = api::GetFolderContent::Create(
//            &stm,
//            [&io_service, this](api::ActionResult result,
//                                api::GetFolderContent::Pointer action)
//            {
//                if (result == api::ActionResult::Failure)
//                {
//                    std::ostringstream ss;
//                    ss << "Failed with error: "
//                       << action->GetErrorCode().message();
//                    BOOST_FAIL(ss.str());
//                }
//                else
//                {
//                    // Should not retrieve folders
//                    BOOST_CHECK_EQUAL(0, action->folders.size());
//
//                    std::cout << "File count: " << action->files.size()
//                              << std::endl;
//
//                    globals::file_count = action->files.size();
//                }
//
//                io_service.stop();
//            },
//            root_folderkey, api::FilesFoldersOrBoth::Files);
//
//    io_service.run();
}

BOOST_AUTO_TEST_CASE(UTGetContentFolders)
{
//    boost::asio::io_service io_service;
//    auto http_config = mf::http::HttpConfig::Create();
//    http_config->SetWorkIoService(&io_service);
//
//    api::SessionMaintainer stm(http_config);
//
//    stm.SetLoginCredentials(api::credentials::Email{username, password});
//
//    auto coro = api::GetFolderContent::Create(
//            &stm,
//            [&io_service, this](api::ActionResult result,
//                                api::GetFolderContent::Pointer action)
//            {
//                if (result == api::ActionResult::Failure)
//                {
//                    std::ostringstream ss;
//                    ss << "Failed with error: "
//                       << action->GetErrorCode().message();
//                    BOOST_FAIL(ss.str());
//                }
//                else
//                {
//                    // Should not retrieve files
//                    BOOST_CHECK_EQUAL(0, action->files.size());
//
//                    std::cout << "Folder count: " << action->folders.size()
//                              << std::endl;
//
//                    globals::folder_count = action->folders.size();
//                }
//
//                io_service.stop();
//            },
//            root_folderkey, api::FilesFoldersOrBoth::Folders);
//
//    io_service.run();
}

BOOST_AUTO_TEST_CASE(UTGetContentFilesAndFolders)
{
//    boost::asio::io_service io_service;
//    auto http_config = mf::http::HttpConfig::Create();
//    http_config->SetWorkIoService(&io_service);
//
//    api::SessionMaintainer stm(http_config);
//
//    stm.SetLoginCredentials(api::credentials::Email{username, password});
//
//    auto coro = api::GetFolderContent::Create(
//            &stm,
//            [&io_service, this](api::ActionResult result,
//                                api::GetFolderContent::Pointer action)
//            {
//                if (result == api::ActionResult::Failure)
//                {
//                    std::ostringstream ss;
//                    ss << "Failed with error: "
//                       << action->GetErrorCode().message();
//                    BOOST_FAIL(ss.str());
//                }
//                else
//                {
//                    // Should not retrieve files
//                    BOOST_CHECK_EQUAL(globals::file_count,
//                                      action->files.size());
//                    BOOST_CHECK_EQUAL(globals::folder_count,
//                                      action->folders.size());
//
//                    std::cout << "File count: " << action->files.size()
//                              << std::endl;
//                    std::cout << "Folder count: " << action->folders.size()
//                              << std::endl;
//                }
//
//                io_service.stop();
//            },
//            root_folderkey, api::FilesFoldersOrBoth::Both);
//
//    io_service.run();
}

BOOST_AUTO_TEST_CASE(UTCloneCloudTree)
{
//    boost::asio::io_service io_service;
//    auto http_config = mf::http::HttpConfig::Create();
//    http_config->SetWorkIoService(&io_service);
//
//    api::SessionMaintainer stm(http_config);
//
//    stm.SetLoginCredentials(api::credentials::Email{username, password});
//
//    std::vector<std::string> folderkeys = {root_folderkey};
//
//
//    auto coro = api::CloneCloudTree::Create(
//            &stm,
//            [&io_service](api::ActionResult result,
//                          api::CloneCloudTree::Pointer action)
//            {
//                if (result == api::ActionResult::Failure)
//                {
//                    std::cout << "Failed. " << action->GetErrorCode().message()
//                              << ": " << action->GetErrorDescription()
//                              << std::endl;
//                }
//                else
//                {
//                    std::cout << "CloneCloudTree done." << std::endl;
//                }
//
//                io_service.stop();
//            },
//            folderkeys);
//
//    io_service.run();
}
