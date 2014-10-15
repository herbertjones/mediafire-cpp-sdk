/**
 * @file MfApiBridgeHelper.cpp
 * @author Zachary Marquez
 * @date 10/14/14
 * @copyright Copyright 2014 MediaFire, LLC. All rights reserved.
 */
#include "MfApiBridgeHelper.hpp"

#include <iostream>

#include <boost/variant/get.hpp>

#include "mediafire_sdk/api/user/get_info.hpp"

#include "MfApiBridgeImpl.hpp"

namespace detail {

MfApiBridgeHelper::MfApiBridgeHelper(MfApiBridgeImpl* owner) :
    owner_(owner),
    api_io_service_(),
    api_work_(new boost::asio::io_service::work(api_io_service_)),
    api_thread_(std::bind((size_t(boost::asio::io_service::*)()) &boost::asio::io_service::run, &api_io_service_)),
    http_config_(mf::http::HttpConfig::Create(&api_io_service_, &api_io_service_)),
    stm_(new mf::api::SessionMaintainer(http_config_)),
    pendingLogin_()
{
    using std::placeholders::_1;
    stm_->SetSessionStateChangeCallback(std::bind(
            &MfApiBridgeHelper::HandleSessionStateChange,
            this,
            _1
        ));
}

MfApiBridgeHelper::~MfApiBridgeHelper()
{
    api_work_.reset();
    stm_.reset();  // Without this the thread join would block indefinitely
    try {
        api_thread_.join();
    }
    catch (std::system_error&)
    {
        // Could not join- Continue destruction but will likely crash...
        std::cerr << "MfApiBridgeHelper: Could not join api_thread_ in destructor!" << std::endl;
    }
}

void MfApiBridgeHelper::LogIn(
        std::string email,
        std::string password,
        LoginCallback onSuccess,
        FailureCallback onFailure
    )
{
    pendingLogin_ = PendingLogin{onSuccess, onFailure};
    stm_->SetLoginCredentials(mf::api::credentials::Email{email,password});
}

void MfApiBridgeHelper::GetUserInfo(
        GetUserInfoCallback onSuccess,
        FailureCallback onFailure
    )
{
    mf::api::user::get_info::Request request;
    stm_->Call(
            request,
            [onSuccess,onFailure](mf::api::user::get_info::Response response)
            {
                if ( response.error_code )
                {
                    onFailure(
                        response.error_code.value(),
                        get_optional_value_or(
                            response.error_string,
                            "No error string"
                        )
                    );
                }
                else
                {
                    UserInfo userInfo{
                        response.email,
                        response.first_name,
                        response.last_name
                        };
                    onSuccess(userInfo);
                }
            },
            &api_io_service_
        );
}

void MfApiBridgeHelper::GetFolderContent(
        const std::string& folderkey,
        ContentType contentType,
        int chunk,
        GetFolderContentCallback onSuccess,
        FailureCallback onFailure
    )
{
    using namespace mf::api::folder;
    get_content::ContentType apiContentType =
        ( contentType == ContentType::File ?
            get_content::ContentType::Files :
            get_content::ContentType::Folders
        );
    get_content::Request request(folderkey, chunk, apiContentType);
    stm_->Call(
            request,
            [onSuccess,onFailure](get_content::Response response)
            {
                if ( response.error_code )
                {
                    onFailure(
                        response.error_code.value(),
                        get_optional_value_or(
                            response.error_string,
                            "No error string"
                        )
                    );
                }
                else if ( response.content_type == get_content::ContentType::Files )
                {
                    MfApiBridgeHelper::HandleGetFolderContentFiles(
                            onSuccess,
                            response
                        );
                }
                else
                {
                    MfApiBridgeHelper::HandleGetFolderContentFolders(
                            onSuccess,
                            response
                        );
                }
            },
            &api_io_service_
        );
}

void MfApiBridgeHelper::HandleGetFolderContentFiles(
        GetFolderContentCallback onSuccess,
        mf::api::folder::get_content::Response& response
    )
{
    using namespace mf::api::folder;
    FolderContents contents;
    for (const get_content::Response::File& file : response.files)
    {
        std::string privacyString =
            ( file.privacy == get_content::Privacy::Public ?
                "Public" : "Private" );
        ContentItem item{
            file.filename,
            ContentType::File,
            file.quickkey,
            to_iso_string(file.created_datetime),
            privacyString
            };
        contents.emplace_back(std::move(item));
    }
    onSuccess(
            contents,
            response.chunk_number,
            response.chunks_remaining == get_content::ChunksRemaining::MoreChunks
        );
}

void MfApiBridgeHelper::HandleGetFolderContentFolders(
        GetFolderContentCallback onSuccess,
        mf::api::folder::get_content::Response& response
    )
{
    using namespace mf::api::folder;
    FolderContents contents;
    for (const get_content::Response::Folder& folder : response.folders)
    {
        std::string privacyString =
            ( folder.privacy == get_content::Privacy::Public ?
                "Public" : "Private" );
        ContentItem item{
            folder.name,
            ContentType::Folder,
            folder.folderkey,
            to_iso_string(folder.created_datetime),
            privacyString
            };
        contents.emplace_back(std::move(item));
    }
    onSuccess(
            contents,
            response.chunk_number,
            response.chunks_remaining == get_content::ChunksRemaining::MoreChunks
        );
}

void MfApiBridgeHelper::HandleSessionStateChange(
        mf::api::SessionState new_session_state
    )
{
    // I would recommend using boost::apply_visitor if you need much more
    // complicated logic than this.
    mf::api::session_state::Running* running_state =
        boost::get<mf::api::session_state::Running>(&new_session_state);
    mf::api::session_state::CredentialsFailure* failure_state =
        boost::get<mf::api::session_state::CredentialsFailure>(&new_session_state);
    if ( running_state )
    {
        CredentialsSuccess();
    }
    else if ( failure_state )
    {
        CredentialsFailure(*failure_state);
    }
    // else State change we don't care about
}

void MfApiBridgeHelper::CredentialsFailure(const mf::api::session_state::CredentialsFailure& credentialsFailure)
{
    if ( pendingLogin_ )
    {
        FailureCallback callback = pendingLogin_->failureCallback;
        pendingLogin_.reset();
        callback(
                credentialsFailure.error_code.value(),
                get_optional_value_or(
                    credentialsFailure.session_token_response.error_string,
                    "No error text"
                )
            );
    }
}

void MfApiBridgeHelper::CredentialsSuccess()
{
    if ( pendingLogin_ )
    {
        LoginCallback callback = pendingLogin_->loginCallback;
        pendingLogin_.reset();
        callback();
    }
}

}  // namespace detail
