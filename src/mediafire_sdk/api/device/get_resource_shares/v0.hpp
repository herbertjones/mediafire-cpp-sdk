/**
 * @file api/device/get_resource_shares.hpp
 * @brief API request: /api/device/get_resource_shares
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
#pragma once

#include <string>
#include <vector>

#include "mediafire_sdk/http/shared_buffer.hpp"
#include "mediafire_sdk/http/headers.hpp"
#include "mediafire_sdk/api/response_base.hpp"

#include "boost/date_time/posix_time/ptime.hpp"

namespace mf {
namespace api {
/** API action path "device" */
namespace device {
/** API action "device/get_resource_shares" */
namespace get_resource_shares {
/** API path "/api/device/get_resource_shares" */
namespace v0 {

enum class Permissions
{
    /** API value "1" */
    Read,
    /** API value "2" */
    ReadWrite,
    /** API value "4" */
    Manage
};

/**
 * @class Response
 * @brief Response from API request "device/get_resource_shares"
 */
class Response : public ResponseBase
{
public:
    struct Share
    {
        /** API response field "contact_key" */
        uint32_t contact_key;

        /** API response field "contact_type" */
        uint32_t contact_type;

        /** API response field "contact_indirect" */
        uint32_t contact_indirect;

        /** API response field "display_name" */
        std::string display_name;

        /** API response field "avatar" */
        boost::optional<std::string> avatar;

        /** API response field "permissions" */
        Permissions share_permissions;
    };
    /** API response field "response.shares" */
    std::vector<Share> shares;
};

class Impl;

/**
 * @class Request
 * @brief Make API request "device/get_resource_shares"
 */
class Request
{
public:
    /**
     * API request "device/get_resource_shares"
     *
     * @param quickkey API parameter "quick_key"
     * @param source_revision API parameter "source_revision"
     * @param target_revision API parameter "target_revision"
     */
    Request(
            std::string quickkey,
            uint32_t source_revision,
            uint32_t target_revision
        );

    // Remaining functions are for use by API library only. --------------------

    /** Requester/SessionMaintainer expected type. */
    typedef Response ResponseType;

    /** Requester/SessionMaintainer expected type. */
    typedef std::function< void( const ResponseType & data)> CallbackType;

    /** Requester/SessionMaintainer expected type. */
    void SetCallback( CallbackType callback_function );

    /** Requester expected method. */
    void HandleContent(
            const std::string & url,
            const mf::http::Headers & headers,
            const std::string & content
        );

    /** Requester expected method. */
    void HandleError(
            const std::string & url,
            std::error_code ec,
            const std::string & error_string
        );

    /** Requester expected method. */
    std::string Url(const std::string & hostname) const;

    /** Requester optional method. */
    mf::http::SharedBuffer::Pointer GetPostData();

    /** SessionMaintainer expected method. */
    void SetSessionToken(
            std::string session_token,
            std::string time,
            int secret_key
        );

    // Enums in class namespace for usage with templates
    using Permissions = enum Permissions;

private:
    std::shared_ptr<Impl> impl_;
};
}  // namespace v0

}  // namespace get_resource_shares
}  // namespace device
}  // namespace api
}  // namespace mf
