/**
 * @file api/upload/get_web_uploads.hpp
 * @brief API request: /api/1.3/upload/get_web_uploads
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
/** API action path "upload" */
namespace upload {
/** API action "upload/get_web_uploads" */
namespace get_web_uploads {
/** API path "/api/1.3/upload/get_web_uploads" */
namespace v1_3 {

enum class Filter
{
    /** API value "no" */
    ActiveOnly,
    /** API value "yes" */
    ActiveAndInactive
};

enum class Activity
{
    /** API value "no" */
    Inactive,
    /** API value "yes" */
    Active
};

enum class Status
{
    /** API value "" */
    Unknown,
    /** API value "1" */
    Entered,
    /** API value "2" */
    Started,
    /** API value "3" */
    Transferring,
    /** API value "4" */
    Downloaded,
    /** API value "5" */
    VerificationWaiting,
    /** API value "6" */
    Verifying,
    /** API value "99" */
    Complete
};

/**
 * @class Response
 * @brief Response from API request "upload/get_web_uploads"
 */
class Response : public ResponseBase
{
public:
    struct WebUpload
    {
        /** API response field "uploadkey" */
        std::string uploadkey;

        /** API response field "active" */
        Activity activity;

        /** API response field "quickkey" */
        boost::optional<std::string> quickkey;

        /** API response field "filename" */
        std::string filename;

        /** API response field "created" */
        boost::optional<boost::posix_time::ptime> created_datetime;

        /** On completion, success or failure depends on the error code being 0.
         */
        Status status_code;

        /** API response field "status" */
        std::string status_text;

        /** API response field "error_status" */
        int32_t error_code;

        /** API response field "url" */
        std::string url;

        /** Textual description of time remaining. */
        std::string eta;

        /** API response field "size" */
        boost::optional<uint64_t> filesize;

        /** API response field "percentage" */
        int32_t percent_complete;
    };
    /** List of web uploads. */
    std::vector<WebUpload> web_uploads;
};

class Impl;

/**
 * @class Request
 * @brief Make API request "upload/get_web_uploads"
 */
class Request
{
public:
    /**
     * API request "upload/get_web_uploads"
     */
    Request();

    /**
     * Optional API parameter "all_web_uploads"
     *
     * @param filter Response may contain only requests actively downloading, or
     *               all in the queue.  The default is to return only active
     *               requests.
     */
    void SetFilter(Filter filter);

    /**
     * Optional API parameter "upload_key"
     *
     * @param upload_key Limit response to a single upload key.  The filter is
     *                   still active, which may filter the response if not set
     *                   to ActiveAndInactive.
     */
    void SetUploadKey(std::string upload_key);

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
private:
    std::shared_ptr<Impl> impl_;
};
}  // namespace v1_3

}  // namespace get_web_uploads
}  // namespace upload
}  // namespace api
}  // namespace mf