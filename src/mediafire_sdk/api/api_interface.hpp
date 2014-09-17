/**
 * @file api_interface.hpp
 * @author Herbert Jones
 * @brief Interface for API classes. Not to be used by any class.
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This is merely a reference for any API class to meet. There is no need to
 * use this as a base class as the Requester is template based.
 */
#pragma once

#include <string>
#include <functional>

#include "mediafire_sdk/http/headers.hpp"
#include "mediafire_sdk/http/http_request.hpp"
#include "mediafire_sdk/api/response_base.hpp"

namespace mf {
namespace api {
namespace example {

/**
 * @class Response
 * @brief Example Response class
 *
 * This class holds the API response data which includes any error that may have
 * occurred.
 */
class Response : public ResponseBase
{
    /** Custom members here */
};

/**
 * @class Request
 * @brief Example code on an API request.
 *
 * This is only an example and no APIs use it as a parent.
 */
class Request
{
public:
    virtual ~Request() {}

    /** Requester/SessionMaintainer expected typedef. */
    typedef Response ResponseType;

    /** Requester/SessionMaintainer expected typedef. */
    typedef std::function<
        void(
                const ResponseType & data
            )> CallbackType;

    /** Requester/SessionMaintainer expected typedef. */
    virtual void SetCallback( CallbackType callback_function ) = 0;

    /** Requester expected method. */
    virtual std::string Url(std::string hostname) const = 0;

    /** Requester expected method. */
    virtual void HandleContent(
            const std::string & url,
            const mf::http::Headers & headers,
            const std::string & content) = 0;

    /** Requester expected method. */
    virtual void HandleError(
            const std::string & url,
            std::error_code ec,
            const std::string & error_string
        ) = 0;

    /** Requester optional method. */
    virtual mf::http::SharedBuffer::Pointer GetPostData() = 0;

    /** Requester optional method. */
    virtual std::shared_ptr<mf::http::PostDataPipeInterface>
        GetPostDataPipe() = 0;

    /** SessionMaintainer optional method. */
    virtual void SetSessionToken(
            std::string session_token,
            std::string time,
            int secret_key
        ) = 0;

private:
};

}  // namespace example
}  // namespace api
}  // namespace mf
