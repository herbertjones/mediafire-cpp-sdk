#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/actions/get_foreign_changes_device.hpp"

#include "mediafire_sdk/api/device/get_foreign_changes.hpp"
#include "mediafire_sdk/api/device/get_status.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE GetForeignChangesDevice
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

BOOST_AUTO_TEST_CASE(UTGetForeignChangesDevice)
{
    boost::asio::io_service io_service;
    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    api::SessionMaintainer stm(http_config);

    stm.SetLoginCredentials(api::credentials::Email{username, password});

    using GetForeignChangesDeviceType = mf::api::
            GetForeignChangesDevice<mf::api::device::get_status::Request,
                                    mf::api::device::get_foreign_changes::
                                            Request>;

    GetForeignChangesDeviceType::CallbackType HandleGetForeignChangesDevice =
            [this, &io_service](
                    const std::vector<GetForeignChangesDeviceType::File> &
                            updated_files,
                    const std::vector<GetForeignChangesDeviceType::Folder> &
                            updated_folders,
                    const std::vector<GetForeignChangesDeviceType::File> &
                            deleted_files,
                    const std::vector<GetForeignChangesDeviceType::Folder> &
                            deleted_folders,
                    const std::vector<GetForeignChangesDeviceType::
                                              DeviceGetStatusErrorType> &
                            get_status_errors,
                    const std::vector<GetForeignChangesDeviceType::
                                              DeviceGetForeignChangesErrorType> &
                            get_changes_errors)
    {
        if (!get_status_errors.empty() || !get_changes_errors.empty())
        {
            BOOST_FAIL("GetForeignChangesDevice reported errors!");
        }
        else
        {
            for (const auto & updated_file : updated_files)
            {
                std::cout << "Updated File: " << updated_file.quickkey
                          << std::endl;
            }

            for (const auto & updated_folder : updated_folders)
            {
                std::cout << "Updated Folder: " << updated_folder.folderkey
                          << std::endl;
            }

            for (const auto & deleted_file : deleted_files)
            {
                std::cout << "Deleted File: " << deleted_file.quickkey
                          << std::endl;
            }

            for (const auto & deleted_folder : deleted_folders)
            {
                std::cout << "Deleted Folder: " << deleted_folder.folderkey
                          << std::endl;
            }

            BOOST_CHECK(!updated_files.empty() || !updated_folders.empty()
                        || !deleted_files.empty() || !deleted_folders.empty());
        }

        io_service.stop();
    };

    auto get_folder_info = GetForeignChangesDeviceType::Create(
            &stm, 193000, "contact_key", std::move(HandleGetForeignChangesDevice));

    get_folder_info->operator()();

    io_service.run();
}
