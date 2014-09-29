/**
 * @file get_limits.cpp
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
// #define OUTPUT_DEBUG
#include "get_limits.hpp"

#include <string>

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/api/session_token_api_base.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace v0 = mf::api::user::get_limits::v0;


#include "mediafire_sdk/api/type_helpers.hpp"

namespace mf {
namespace api {
/** API action path "user" */
namespace user {
namespace get_limits {
namespace v0 {

const std::string api_path("/api/user/get_limits");

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

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.storage_base",
            &response->storage_base ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.storage_base\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.storage_bonus",
            &response->storage_bonus ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.storage_bonus\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.storage_limit",
            &response->storage_limit ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.storage_limit\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.storage_used",
            &response->storage_used ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.storage_used\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.bandwidth_limit",
            &response->bandwidth_limit ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.bandwidth_limit\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.bandwidth_used",
            &response->bandwidth_used ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.bandwidth_used\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.collaboration_limit",
            &response->collaboration_limit ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.collaboration_limit\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.collaboration_today",
            &response->collaboration_today ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.collaboration_today\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.one_time_downloads_limit",
            &response->one_time_downloads_limit ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.one_time_downloads_limit\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.one_time_downloads_today",
            &response->one_time_downloads_today ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.one_time_downloads_today\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.streaming_bandwidth_limit",
            &response->streaming_bandwidth_limit ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.streaming_bandwidth_limit\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.streaming_bandwidth_today",
            &response->streaming_bandwidth_today ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.streaming_bandwidth_today\"");

    // create_content_parse_single required
    if ( ! GetIfExists(
            response->pt,
            "response.upload_size_limit",
            &response->upload_size_limit ) )
        return_error(
            mf::api::api_code::ContentInvalidData,
            "missing \"response.upload_size_limit\"");

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

}  // namespace v0
}  // namespace get_limits
}  // namespace user
}  // namespace api
}  // namespace mf
