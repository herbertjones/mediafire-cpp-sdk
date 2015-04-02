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

BOOST_AUTO_TEST_CASE(UTGetContentFiles)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    using RequestType = mf::api::folder::get_content::Request;
    using ResponseType = RequestType::ResponseType;
    using ResponseTypeFiles = std::vector<ResponseType::File>;
    using ResponseTypeFolders = std::vector<ResponseType::Folder>;

    using GetFolderContentType = mf::api::GetFolderContents<RequestType>;

    GetFolderContentType::CallbackType HandleGetFolderContents =
            [this, &io_service](
                    const ResponseTypeFiles & response_files,
                    const ResponseTypeFolders & response_folders,
                    const std::vector<GetFolderContentType::ErrorType> & errors)
    {
        if (!errors.empty())
            BOOST_FAIL("GetFolderContents returned errors");

        BOOST_CHECK(response_folders.empty());

        io_service.stop();
    };

    auto get_folder_contents = GetFolderContentType::Create(
            &stm,
            "",
            GetFolderContentType::FilesOrFoldersOrBoth::Files,
            std::move(HandleGetFolderContents));
    get_folder_contents->operator()();

    io_service.run();
}

BOOST_AUTO_TEST_CASE(UTGetContentFolders)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    using RequestType = mf::api::folder::get_content::Request;
    using ResponseType = RequestType::ResponseType;
    using ResponseTypeFiles = std::vector<ResponseType::File>;
    using ResponseTypeFolders = std::vector<ResponseType::Folder>;

    using GetFolderContentType = mf::api::GetFolderContents<RequestType>;

    GetFolderContentType::CallbackType HandleGetFolderContents =
            [this, &io_service](
                    const ResponseTypeFiles & response_files,
                    const ResponseTypeFolders & response_folders,
                    const std::vector<GetFolderContentType::ErrorType> & errors)
    {
        if (!errors.empty())
            BOOST_FAIL("GetFolderContents returned errors");

        BOOST_CHECK(response_files.empty());

        io_service.stop();
    };

    auto get_folder_contents = GetFolderContentType::Create(
            &stm,
            "",
            GetFolderContentType::FilesOrFoldersOrBoth::Folders,
            std::move(HandleGetFolderContents));
    get_folder_contents->operator()();

    io_service.run();
}

BOOST_AUTO_TEST_CASE(UTGetContentFilesAndFolders)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    using RequestType = mf::api::folder::get_content::Request;
    using ResponseType = RequestType::ResponseType;
    using ResponseTypeFiles = std::vector<ResponseType::File>;
    using ResponseTypeFolders = std::vector<ResponseType::Folder>;

    using GetFolderContentType = mf::api::GetFolderContents<RequestType>;

    GetFolderContentType::CallbackType HandleGetFolderContents =
            [this, &io_service](
                    const ResponseTypeFiles & response_files,
                    const ResponseTypeFolders & response_folders,
                    const std::vector<GetFolderContentType::ErrorType> & errors)
    {
        if (!errors.empty())
            BOOST_FAIL("GetFolderContents returned errors");

        io_service.stop();
    };

    auto get_folder_contents = GetFolderContentType::Create(
            &stm,
            "",
            GetFolderContentType::FilesOrFoldersOrBoth::Both,
            std::move(HandleGetFolderContents));
    get_folder_contents->operator()();

    io_service.run();
}

BOOST_AUTO_TEST_CASE(UTCloneCloudTree)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    using CloneCloudTreeType
            = mf::api::CloneCloudTree<mf::api::folder::get_content::Request>;

    CloneCloudTreeType::CallbackType HandleCloneCloudTree = [this, &io_service](
            const std::vector<CloneCloudTreeType::File> & response_files,
            const std::vector<CloneCloudTreeType::Folder> & response_folders,
            const std::vector<CloneCloudTreeType::ErrorType> & errors)
    {
        if (!errors.empty())
            BOOST_FAIL("GetFolderContents returned errors");

        std::cout << "File Count: " << response_files.size() << std::endl;
        std::cout << "Folder count: " << response_folders.size() << std::endl;

        io_service.stop();
    };

    // Setup the work manager to limit the number of things posted onto
    // io_service.
    auto work_manager = mf::api::WorkManager::Create(&io_service);
    work_manager->SetMaxConcurrentWork(10);
//
    auto clone_cloud_tree = CloneCloudTreeType::Create(
            &stm, "", work_manager, std::move(HandleCloneCloudTree));
    clone_cloud_tree->operator()();

    io_service.run();
}
