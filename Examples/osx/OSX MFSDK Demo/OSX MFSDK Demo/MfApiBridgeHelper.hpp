/**
 * @file MfApiBridgeHelper.hpp
 * @author Zachary Marquez
 * @date 10/14/14
 * @copyright Copyright 2014 MediaFire, LLC. All rights reserved.
 */
#pragma once

#include <functional>
#include <thread>

#include <boost/asio.hpp>
#include <boost/optional.hpp>

#include "mediafire_sdk/api/folder/get_content.hpp"
#include "mediafire_sdk/api/session_maintainer.hpp"

#include "MfApiBridgeTypes.hpp"

namespace detail {

struct PendingLogin
{
    LoginCallback loginCallback;
    FailureCallback failureCallback;
};

class MfApiBridgeImpl;

/*
 * This class actually makes all of the MFSDK calls and translates from  MFSDK
 * types to the C++ types defined in MfApiBridgeTypes.h.
 *
 * This class demonstrates correct usage of the MediaFire SDK and can be used
 * as a starting point for your own application. Keep in mind that this
 * abstraction is not necessary in pure C++ code.
 *
 * Note: All callbacks from the API layer happen on the API thread. We could
 *       have created a separate io_service for such calls but this was not
 *       necessary for a simple demo.
 */
class MfApiBridgeHelper
{
public:
    MfApiBridgeHelper(MfApiBridgeImpl* owner);
    ~MfApiBridgeHelper();

    void LogIn(
            std::string email,
            std::string password,
            LoginCallback onSuccess,
            FailureCallback onFailure
        );

    void GetUserInfo(
            GetUserInfoCallback onSuccess,
            FailureCallback onFailure
        );

    void GetFolderContent(
            const std::string& folderkey,
            ContentType contentType,
            int chunk,
            GetFolderContentCallback onSuccess,
            FailureCallback onFailure
        );

private:
    void HandleSessionStateChange(
            mf::api::SessionState new_session_state
        );

    static void HandleGetFolderContentFiles(
            GetFolderContentCallback onSuccess,
            mf::api::folder::get_content::Response& response
        );
    static void HandleGetFolderContentFolders(
            GetFolderContentCallback onSuccess,
            mf::api::folder::get_content::Response& response
        );

    void CredentialsFailure(const mf::api::session_state::CredentialsFailure& credentialsFailure);
    void CredentialsSuccess();

    // Parent
    MfApiBridgeImpl* owner_;

    // Api io_service
    boost::asio::io_service api_io_service_;
    std::unique_ptr<boost::asio::io_service::work> api_work_;

    // Api thread
    std::thread api_thread_;

    // Config for HTTP requests
    mf::http::HttpConfig::Pointer http_config_;

    // Cloud API communication
    std::unique_ptr<mf::api::SessionMaintainer> stm_;

    // Pending login attempt
    boost::optional<PendingLogin> pendingLogin_;
};

}  // namespace detail
