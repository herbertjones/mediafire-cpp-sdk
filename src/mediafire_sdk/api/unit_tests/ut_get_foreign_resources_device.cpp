#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/actions/get_foreign_resources_device.hpp"

#include "mediafire_sdk/api/device/get_foreign_resources.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE GetForeignResourcesDevice
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

    using GetForeignResourcesDeviceType = mf::api::
            GetForeignResourcesDevice<mf::api::device::get_foreign_resources::
                                              Request>;
    using File = GetForeignResourcesDeviceType::File;
    using Folder = GetForeignResourcesDeviceType::Folder;
    using ErrorType = GetForeignResourcesDeviceType::ErrorType;

    auto HandleGetForeignResourcesDeviceType =
            [this, &io_service](const std::vector<File> & files,
               const std::vector<Folder> & folders,
               const std::vector<ErrorType> errors)
    {
        BOOST_FAIL(!errors.empty());

        io_service.stop();
    };

    auto get_foreign_resources_device = GetForeignResourcesDeviceType::Create(
            &stm, HandleGetForeignResourcesDeviceType);
    get_foreign_resources_device->operator()();

    io_service.run();
}