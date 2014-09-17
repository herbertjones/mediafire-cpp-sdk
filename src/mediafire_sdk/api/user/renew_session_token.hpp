/**
 * @file renew_session_token.hpp
 * @author Herbert Jones
 * @brief API user/renew_session_token
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>
#include <functional>

#include "mediafire_sdk/http/headers.hpp"
#include "mediafire_sdk/api/response_base.hpp"

namespace mf {
namespace api {
namespace user {
namespace renew_session_token {
namespace v0 {

/**
 * Api user/renew_session_token. To be used with Requester.
 */
class Request
{
public:
    /** Return type via callback. */
    class ResponseType : public ResponseBase
    {
    public:
        /** Renewed session token */
        std::string session_token;
    };

    /** Callback type */
    typedef std::function<
        void(
                const ResponseType & data
            )> CallbackType;

    /** Requester/SessionMaintainer expected method. */
    void SetCallback( CallbackType callback_function )
    {
        callback_ = callback_function;
    };

    /**
     * @brief CTOR
     *
     * @param[in] session_token The existing valid session token.
     */
    Request(
            const std::string & session_token
            );

    /** Requester expected method. */
    std::string Url(std::string hostname) const;

    /** Requester expected method. */
    void HandleContent(
            const std::string & url,
            const mf::http::Headers & headers,
            const std::string & content);

    /** Requester expected method. */
    void HandleError(
            const std::string & url,
            std::error_code ec,
            const std::string & error_string);

private:
    const std::string session_token_;

    CallbackType callback_;
};
typedef Request::ResponseType Response;
}  // namespace v0

// The latest version
using namespace v0;  // NOLINT

}  // namespace renew_session_token
}  // namespace user
}  // namespace api
}  // namespace mf
