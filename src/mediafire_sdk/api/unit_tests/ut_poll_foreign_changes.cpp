#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/actions/poll_foreign_changes.hpp"

#include "mediafire_sdk/api/contact/fetch.hpp"
#include "mediafire_sdk/api/device/get_status.hpp"
#include "mediafire_sdk/api/device/get_foreign_changes.hpp"
#include "mediafire_sdk/api/file/get_info.hpp"
#include "mediafire_sdk/api/folder/get_info.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE PollForeignChanges
#include "boost/test/unit_test.hpp"

#if !defined(TEST_USER_1_USERNAME) || !defined(TEST_USER_1_PASSWORD)
#error "TEST_USER defines not set."
#endif

namespace
{
const std::string username = "hcmftest+1@gmail.com";
const std::string password = "abc123";
}  // end anonymous namespace

namespace api = mf::api;

BOOST_AUTO_TEST_CASE(UTPollForeignChanges)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    using DeviceGetStatusType = mf::api::device::get_status::Request;
    using ContactFetchType = mf::api::contact::fetch::Request;
    using DeviceGetForeignChangesType
            = mf::api::device::get_foreign_changes::Request;
    using FolderGetInfoType = mf::api::folder::get_info::Request;
    using FileGetInfoType = mf::api::file::get_info::Request;

    using PollForeignChangesType
            = mf::api::PollForeignChanges<DeviceGetStatusType,
                                          ContactFetchType,
                                          DeviceGetForeignChangesType,
                                          FolderGetInfoType,
                                          FileGetInfoType>;

    // Response types
    using File = typename PollForeignChangesType::File;
    using Folder = typename PollForeignChangesType::Folder;
    using FolderInfo = typename PollForeignChangesType::FolderInfo;
    using FileInfo = typename PollForeignChangesType::FileInfo;

    // Error types
    using DeviceGetStatusErrorType =
            typename PollForeignChangesType::DeviceGetStatusErrorType;
    using ContactFetchErrorType = PollForeignChangesType::ContactFetchErrorType;
    using DeviceGetForeignChangesErrorType =
            typename PollForeignChangesType::DeviceGetForeignChangesErrorType;
    using GetInfoFileErrorType =
            typename PollForeignChangesType::GetInfoFileErrorType;
    using GetInfoFolderErrorType =
            typename PollForeignChangesType::GetInfoFolderErrorType;

    auto HandlePollForeignChanges = [this, &io_service](
            const std::vector<File> & deleted_files,
            const std::vector<Folder> & deleted_folders,
            const std::vector<FileInfo> & updated_files_info,
            const std::vector<FolderInfo> & updated_folders_info,
            const std::vector<DeviceGetStatusErrorType> & get_status_errors,
            const std::vector<ContactFetchErrorType> & contact_fetch_errors,
            const std::vector<DeviceGetForeignChangesErrorType> &
                    get_changes_errors,
            const std::vector<GetInfoFileErrorType> & get_info_file_errors,
            const std::vector<GetInfoFolderErrorType> & get_info_folder_errors)
    {
        if (!get_status_errors.empty())
            BOOST_FAIL("device/get_status returned errors");

        if (!contact_fetch_errors.empty())
            BOOST_FAIL("contact/fetch returned errors");

        if (!get_changes_errors.empty())
            BOOST_FAIL("device/get_changes returned errors");

        if (!get_info_file_errors.empty())
            BOOST_FAIL("file/get_info returned errors");

        if (!get_info_folder_errors.empty())
            BOOST_FAIL("folder/get_info returned errors");

        for (const auto & updated_file_info : updated_files_info)
        {
            std::cout << "Updated File Info: " << updated_file_info.quickkey
                      << " " << updated_file_info.filename << std::endl;
        }

        for (const auto & updated_folder_info : updated_folders_info)
        {
            std::cout << "Updated Folder Info: "
                      << updated_folder_info.folderkey << " " << updated_folder_info.name << std::endl;
        }

        for (const auto & deleted_file : deleted_files)
        {
            std::cout << "Deleted File: " << deleted_file.quickkey << std::endl;
        }

        for (const auto & deleted_folder : deleted_folders)
        {
            std::cout << "Deleted Folder: " << deleted_folder.folderkey
                      << std::endl;
        }

        io_service.stop();
    };

    auto work_manager = mf::api::WorkManager::Create(&io_service);

    auto poll_foreign_changes = PollForeignChangesType::Create(
            &stm, 0, work_manager, std::move(HandlePollForeignChanges));

    poll_foreign_changes->operator()();

    io_service.run();
}
