/**
 * @file api/file/update_file.hpp
 * @brief API request: /api/file/update_file
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
/** API action "file/update_file" */
namespace update_file {
/** API path "/api/file/update_file" */
namespace v0 {

/**
 * @class Response
 * @brief Response from API request "file/update_file"
 */
class Response : public ResponseBase
{
public:
    /** API response field "response.device_revision" */
    boost::optional<uint32_t> device_revision;
};

class Impl;

/**
 * @class Request
 * @brief Make API request "file/update_file"
 */
class Request
{
public:
    /**
     * API request "file/update_file"
     *
     * @param from_quickkey API parameter "from_quickkey"
     * @param to_quickkey API parameter "to_quickkey"
     */
    Request(
            std::string from_quickkey,
            std::string to_quickkey
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
}  // namespace v0

}  // namespace update_file
}  // namespace file
}  // namespace api
}  // namespace mf
