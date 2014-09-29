/**
 * @file session_token_api_base.hpp
 * @author Herbert Jones
 * @brief Private base class for API request types that need session tokens.
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <map>
#include <string>

#include "api_base.hpp"

namespace mf {
namespace api {
namespace detail {

/**
 * @interface SessionTokenApiBaseStatic
 * @brief Non templated base class for session token API operations.
 *
 * Contains base functionality for API calls which don't need to be regenerated
 * due to return type changing.
 */
class SessionTokenApiBaseStatic
{
public:
    /** SessionMaintainer expected method. */
    void SetSessionToken(
            std::string session_token,
            std::string time,
            int secret_key
        );

protected:
    std::string session_token_;
    std::string time_;
    int secret_key_;

    /** ApiBase expected method. */
    virtual api::RequestMethod GetRequestMethod() const = 0;

    std::string MakePathAndQuery(
            const std::string & url_path,
            const std::map<std::string, std::string> & query_parts
        ) const;
    std::string MakePost(
            const std::string & url_path,
            const std::map<std::string, std::string> & query_parts
        ) const;
    std::string GetSignature(const std::string & path_and_query) const;
};

}  // namespace detail
}  // namespace api
}  // namespace mf
