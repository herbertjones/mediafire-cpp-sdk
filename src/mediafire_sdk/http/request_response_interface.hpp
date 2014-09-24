/**
 * @file request_response_interface.hpp
 * @author Herbert Jones
 * @brief Interface for handling response from HTTP request.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <map>
#include <memory>
#include <string>
#include <system_error>

#include "buffer_interface.hpp"
#include "mediafire_sdk/http/headers.hpp"
#include "mediafire_sdk/http/url.hpp"

namespace mf {
namespace http {

/**
 * Interface for classes making HTTP requests.
 */
class RequestResponseInterface
{
public:
    RequestResponseInterface() {}
    virtual ~RequestResponseInterface() {}

    /**
     * @brief Called after response header is parsed with redirect directions
     * which are being followed.
     *
     * @param[in] raw_header The headers in plain text.
     * @param[in] headers Headers parsed into parts.
     * @param[in] new_url New request target.
     */
    virtual void RedirectHeaderReceived(
            std::string raw_header,
            std::map<std::string, std::string> headers,
            Url new_url
        ) = 0;

    /**
     * @brief Called after response header is parsed.
     *
     * This is called once for the response.  If there are any redirects they
     * are received through RedirectHeaderReceived.
     *
     * @param[in] headers Headers parsed into parts.
     */
    virtual void ResponseHeaderReceived(
            Headers headers
        ) = 0;

    /**
     * @brief Called when content received.
     *
     * As content is streamed from the remote server, this is called with the
     * streamed content.
     *
     * @param[in] start_pos Where in the response content the buffer starts.
     * @param[in] buffer The streamed data from the remote server.
     */
    virtual void ResponseContentReceived(
            size_t start_pos,
            std::shared_ptr<BufferInterface> buffer
        ) = 0;

    /**
     * @brief Called when an error occurs. Completes the request.
     *
     * @param[in] error_code The error code of the error.
     * @param[in] error_text Long description of the error.
     */
    virtual void RequestResponseErrorEvent(
            std::error_code error_code,
            std::string error_text
        ) = 0;

    /**
     * @brief Called when the request is successful. Completes the request.
     */
    virtual void RequestResponseCompleteEvent() = 0;
};

}  // namespace http
}  // namespace mf
