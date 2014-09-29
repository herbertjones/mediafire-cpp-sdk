/**
 * @file tokenless_api_base_static.hpp
 * @author Herbert Jones
 * @brief Base class for API request types that do not need session tokens.
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <map>
#include <string>

namespace mf {
namespace api {
namespace detail {
/**
 * @interface TokenlessApiBaseStatic
 * @brief Non templated base class for session token API operations.
 *
 * Contains base functionality for API calls which don't need to be regenerated
 * due to return type changing.
 */
class TokenlessApiBaseStatic
{
public:

protected:
    std::string MakePathAndQuery(
            const std::string & url_path,
            const std::map<std::string, std::string> & query_parts
        ) const;

    std::string MakePost(
            const std::string & url_path,
            const std::map<std::string, std::string> & query_parts
        ) const;
};

}  // namespace detail
}  // namespace api
}  // namespace mf
