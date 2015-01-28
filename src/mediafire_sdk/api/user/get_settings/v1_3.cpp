/**
 * @file get_settings.cpp
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
// #define OUTPUT_DEBUG
#include "v1_3.hpp"

#include <string>

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/api/session_token_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v1_3 = mf::api::user::get_settings::v1_3;


#include "mediafire_sdk/api/type_helpers.hpp"

namespace mf {
namespace api {
/** API action path "user" */
namespace user {
namespace get_settings {
namespace v1_3 {

const std::string api_path("/api/1.3/user/get_settings");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    Impl();

    virtual void BuildUrl(
        std::string * path,
        std::map<std::string, std::string> * query_parts
    ) const override;

    virtual void ParseResponse( Response * response ) override;

    mf::http::SharedBuffer::Pointer GetPostData();

    mf::api::RequestMethod GetRequestMethod() const
    {
        return mf::api::RequestMethod::Post;
    }
};

Impl::Impl()
{
}

void Impl::BuildUrl(
            std::string * path,
            std::map<std::string, std::string> * /* query_parts */
    ) const
{
    *path = api_path + ".php";
}

void Impl::ParseResponse( Response * response )
{
    // This function uses return defines for readability and maintainability.
#   define return_error(error_type, error_message)                             \
    {                                                                          \
        SetError(response, error_type, error_message);                         \
        return;                                                                \
    }
    response->email_address_validated = EmailValidation::NeedsValidation;
    response->instant_uploads = InstantUploads::Disabled;
    response->show_download_page_me_from_me = DownloadPage::NotSkipped;
    response->show_download_page_me_from_others = DownloadPage::NotSkipped;
    response->show_download_page_others_from_me = DownloadPage::NotSkipped;
    response->auto_bandwidth = AutoBandwidth::NoAction;
    response->used_storage_size = 0;
    response->storage_limit = 0;
    response->storage_limit_exceeded = StorageLimit::InBounds;
    response->previous_file_versions = 0;
    response->share_link_status = ShareLinkStatus::Inherit;
    response->collect_metadata = CollectMetadata::No;

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.settings.validated",
                &optval) )
        {
            if ( optval == "no" )
                response->email_address_validated = EmailValidation::NeedsValidation;
            else if ( optval == "yes" )
                response->email_address_validated = EmailValidation::Validated;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.settings.instant_uploads_enabled",
                &optval) )
        {
            if ( optval == "no" )
                response->instant_uploads = InstantUploads::Disabled;
            else if ( optval == "yes" )
                response->instant_uploads = InstantUploads::Enabled;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.settings.show_download_page.me_from_me",
                &optval) )
        {
            if ( optval == "no" )
                response->show_download_page_me_from_me = DownloadPage::Skipped;
            else if ( optval == "yes" )
                response->show_download_page_me_from_me = DownloadPage::NotSkipped;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.settings.show_download_page.me_from_others",
                &optval) )
        {
            if ( optval == "no" )
                response->show_download_page_me_from_others = DownloadPage::Skipped;
            else if ( optval == "yes" )
                response->show_download_page_me_from_others = DownloadPage::NotSkipped;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.settings.show_download_page.others_from_me",
                &optval) )
        {
            if ( optval == "no" )
                response->show_download_page_others_from_me = DownloadPage::Skipped;
            else if ( optval == "yes" )
                response->show_download_page_others_from_me = DownloadPage::NotSkipped;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.settings.auto_bandwidth.enabled",
                &optval) )
        {
            if ( optval == "no" )
                response->auto_bandwidth = AutoBandwidth::NoAction;
            else if ( optval == "yes" )
                response->auto_bandwidth = AutoBandwidth::PurchaseMoreAutomatically;
        }
    }

    // create_content_parse_single optional with default
    GetIfExists(
            response->pt,
            "response.settings.used_storage_size",
            &response->used_storage_size);

    // create_content_parse_single optional with default
    GetIfExists(
            response->pt,
            "response.settings.storage_limit",
            &response->storage_limit);

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.settings.storage_limit_exceeded",
                &optval) )
        {
            if ( optval == "no" )
                response->storage_limit_exceeded = StorageLimit::InBounds;
            else if ( optval == "yes" )
                response->storage_limit_exceeded = StorageLimit::Exceeded;
        }
    }

    // create_content_parse_single optional with default
    GetIfExists(
            response->pt,
            "response.settings.previous_file_versions",
            &response->previous_file_versions);

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.settings.share_link_status",
                &optval) )
        {
            if ( optval == "disabled" )
                response->share_link_status = ShareLinkStatus::Disabled;
            else if ( optval == "enabled" )
                response->share_link_status = ShareLinkStatus::Enabled;
            else if ( optval == "inherit" )
                response->share_link_status = ShareLinkStatus::Inherit;
        }
    }

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.settings.collect_metadata",
                &optval) )
        {
            if ( optval == "no" )
                response->collect_metadata = CollectMetadata::No;
            else if ( optval == "yes" )
                response->collect_metadata = CollectMetadata::Yes;
        }
    }

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;


    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\n");
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request() :
    impl_(new Impl())
{
}

void Request::SetCallback( CallbackType callback_function )
{
    impl_->SetCallback(callback_function);
}

void Request::HandleContent(
        const std::string & url,
        const mf::http::Headers & headers,
        const std::string & content
    )
{
    impl_->HandleContent(url, headers, content);
}

void Request::HandleError(
        const std::string & url,
        std::error_code ec,
        const std::string & error_string
    )
{
    impl_->HandleError(url, ec, error_string);
}

std::string Request::Url(const std::string & hostname) const
{
    return impl_->Url(hostname);
}

void Request::SetSessionToken(
        std::string session_token,
        std::string time,
        int secret_key
    )
{
    impl_->SetSessionToken(session_token, time, secret_key);
}

mf::http::SharedBuffer::Pointer Request::GetPostData()
{
    return impl_->GetPostData();
}

}  // namespace v1_3
}  // namespace get_settings
}  // namespace user
}  // namespace api
}  // namespace mf