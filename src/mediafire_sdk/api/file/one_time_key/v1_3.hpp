/**
 * @file api/file/one_time_key.hpp
 * @brief API request: /api/1.3/file/one_time_key
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
/** API action path "file" */
namespace file {
/** API action "file/one_time_key" */
namespace one_time_key {
/** API path "/api/1.3/file/one_time_key" */
namespace v1_3 {

enum class NofifyOwnerByEmail
{
    /** API value "yes" */
    Yes,
    /** API value "no" */
    No
};

enum class BurnAfterUse
{
    /** API value "no" */
    No,
    /** API value "yes" */
    Yes
};

enum class GetCountsOnly
{
    /** API value "yes" */
    Yes,
    /** API value "no" */
    No
};

/**
 * @class Response
 * @brief Response from API request "file/one_time_key"
 */
class Response : public ResponseBase
{
public:
    Response() :
        one_time_key_request_count(0),
        one_time_key_request_max_count(0)
    {}
    /** API response field "response.one_time_key_request_count" */
    uint32_t one_time_key_request_count;

    /** API response field "response.one_time_key_request_max_count" */
    uint32_t one_time_key_request_max_count;

    /** This token can be used to reconfigure the key. */
    std::string token;

    /** API response field "response.links.download" */
    boost::optional<std::string> download_link;

    /** API response field "response.links.view" */
    boost::optional<std::string> view_link;

    /** API response field "response.links.watch" */
    boost::optional<std::string> watch_link;

    /** API response field "response.links.listen" */
    boost::optional<std::string> listen_link;
};

class Impl;

/**
 * @class Request
 * @brief Make API request "file/one_time_key"
 */
class Request
{
public:
    /**
     * API request "file/one_time_key"
     */
    Request();

    /**
     * Optional API parameter "quick_key"
     *
     * @param quickkey Single file key(quickkey) to associate with new one-time
     *                 download key.
     */
    void SetQuickkey(std::string quickkey);

    /**
     * Optional API parameter "get_counts_only"
     *
     * @param get_counts_only Only return the request counts.
     */
    void SetGetCountsOnly(GetCountsOnly get_counts_only);

    /**
     * Optional API parameter "duration"
     *
     * @param duration_minutes The number of minutes before the one-time key
     *                         expires without use.  Default duration is 30
     *                         days.
     */
    void SetDurationMinutes(uint32_t duration_minutes);

    /**
     * Optional API parameter "email_notification"
     *
     * @param nofify_owner_on_access Notify the file owner by email when the
     *                               one-time download link is accessed.
     */
    void SetNofifyOwnerOnAccess(NofifyOwnerByEmail nofify_owner_on_access);

    /**
     * Optional API parameter "success_callback_url"
     *
     * @param success_callback_url An absolute URL which is called when the user
     *                             successfully downloads the file.
     */
    void SetSuccessCallbackUrl(std::string success_callback_url);

    /**
     * Optional API parameter "error_callback_url"
     *
     * @param error_callback_url An absolute URL which is called when the
     *                           download fails.
     */
    void SetErrorCallbackUrl(std::string error_callback_url);

    /**
     * Optional API parameter "bind_ip"
     *
     * @param bind_ip A comma-separated list of IP masks/ranges to restrict the
     *                download to matching user IP addresses. (e.g.
     *                '68.154.11.0/8, 145.230.230.115-145.230.240.33,
     *                78.192.10.10')
     */
    void SetBindIp(std::string bind_ip);

    /**
     * Optional API parameter "burn_after_use"
     *
     * @param burn_after_use Invalidate the one-time download link after first
     *                       use. If bind_ip is not passed this parameter is
     *                       ignored.  Default is to invalidate.
     */
    void SetBurnAfterUse(BurnAfterUse burn_after_use);

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
    using NofifyOwnerByEmail = enum NofifyOwnerByEmail;
    using BurnAfterUse = enum BurnAfterUse;
    using GetCountsOnly = enum GetCountsOnly;

private:
    std::shared_ptr<Impl> impl_;
};
}  // namespace v1_3

}  // namespace one_time_key
}  // namespace file
}  // namespace api
}  // namespace mf
