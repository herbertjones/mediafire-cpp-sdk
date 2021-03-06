/**
 * @file api/upload/poll_upload.hpp
 * @brief API request: /api/1.3/upload/poll_upload
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
/** API action "upload/poll_upload" */
namespace poll_upload {
/** API path "/api/1.3/upload/poll_upload" */
namespace v1_3 {

enum class AllUnitsReady
{
    /** API value "no" */
    No,
    /** API value "yes" */
    Yes
};

/**
 * @class Response
 * @brief Response from API request "upload/poll_upload"
 */
class Response : public ResponseBase
{
public:
    Response() :
        fileerror(0)
    {}
    struct ResumableData
    {
        /** API response field "number_of_units" */
        uint32_t number_of_units;

        /** API response field "all_units_ready" */
        AllUnitsReady all_units_ready;

        /** API response field "unit_size" */
        uint64_t unit_size;

        /** API response field "bitmap.count" */
        boost::optional<uint32_t> bitmap_count;

        /** API response field "bitmap.words" */
        std::vector<uint16_t> words;

        /** API response field "upload_key" */
        boost::optional<std::string> upload_key;
    };
    /** API response field "response.doupload.result" */
    int32_t result;

    /** API response field "response.doupload.fileerror" */
    int32_t fileerror;

    /** API response field "response.doupload.created" */
    boost::optional<boost::posix_time::ptime> created;

    /** API response field "response.doupload.description" */
    boost::optional<std::string> description;

    /** New filename if filename was changed. */
    boost::optional<std::string> filename;

    /** API response field "response.doupload.hash" */
    boost::optional<std::string> hash;

    /** API response field "response.doupload.quickkey" */
    boost::optional<std::string> quickkey;

    /** API response field "response.doupload.resumable_upload" */
    boost::optional<ResumableData> resumable;

    /** API response field "response.doupload.revision" */
    boost::optional<uint32_t> revision;

    /** API response field "response.doupload.size" */
    boost::optional<uint64_t> filesize;

    /** API response field "response.doupload.status" */
    boost::optional<int32_t> status;
};

class Impl;

/**
 * @class Request
 * @brief Make API request "upload/poll_upload"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using AllUnitsReady = enum AllUnitsReady;

    /**
     * API request "upload/poll_upload"
     *
     * @param key API parameter "key"
     */
    explicit Request(
            std::string key
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
private:
    std::shared_ptr<Impl> impl_;
};
}  // namespace v1_3

}  // namespace poll_upload
}  // namespace upload
}  // namespace api
}  // namespace mf
