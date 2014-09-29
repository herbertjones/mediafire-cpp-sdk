/**
 * @file copy.cpp
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
// #define OUTPUT_DEBUG
#include "copy.hpp"

#include <string>

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/api/session_token_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v0 = mf::api::folder::copy::v0;


#include "mediafire_sdk/api/type_helpers.hpp"

namespace mf {
namespace api {
/** API action path "folder" */
namespace folder {
namespace copy {
namespace v0 {

const std::string api_path("/api/folder/copy");

// Impl ------------------------------------------------------------------------

class Impl : public SessionTokenApiBase<Response>
{
public:
    Impl(
            std::string folderkey,
            std::string target_parent_folderkey
        );

    std::string folderkey_;
    std::string target_parent_folderkey_;
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

Impl::Impl(
        std::string folderkey,
        std::string target_parent_folderkey
    ) :
    folderkey_(folderkey),
    target_parent_folderkey_(target_parent_folderkey)
{
}

void Impl::BuildUrl(
            std::string * path,
            std::map<std::string, std::string> * query_parts
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
    response->asynchronous = Asynchronous::Synchronous;

    {
        std::string optval;
        // create_content_enum_parse TSingle
        if ( GetIfExists(
                response->pt,
                "response.asynchronous",
                &optval) )
        {
            if ( optval == "no" )
                response->asynchronous = Asynchronous::Synchronous;
            else if ( optval == "yes" )
                response->asynchronous = Asynchronous::Asynchronous;
        }
    }

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.device_revision",
            &response->device_revision ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.device_revision\"");

    // create_content_parse_array_front required
    if ( ! GetIfExistsArrayFront(
            response->pt,
            "response.new_folderkeys",
            &response->folderkey ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.new_folderkeys\"");

#   undef return_error
}

mf::http::SharedBuffer::Pointer Impl::GetPostData()
{
    std::map<std::string, std::string> parts;

    parts["folder_key_src"] = folderkey_;
    parts["folder_key_dst"] = target_parent_folderkey_;

    std::string post_data = MakePost(api_path + ".php", parts);
    AddDebugText(" POST data: " + post_data + "\n");
    return mf::http::SharedBuffer::Create(post_data);
}

// Request ---------------------------------------------------------------------

Request::Request(
        std::string folderkey,
        std::string target_parent_folderkey
    ) :
    impl_(new Impl(folderkey, target_parent_folderkey))
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

}  // namespace v0
}  // namespace copy
}  // namespace folder
}  // namespace api
}  // namespace mf
