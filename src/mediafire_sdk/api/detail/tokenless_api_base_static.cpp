/**
 * @file tokenless_api_base.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "tokenless_api_base_static.hpp"

#include <map>
#include <string>
#include <utility>

#include "mediafire_sdk/utils/url_encode.hpp"

namespace mf {
namespace api {
namespace detail {

std::string TokenlessApiBaseStatic::MakePathAndQuery(
        const std::string & url_path,
        const std::map<std::string, std::string> & query_parts
    ) const
{
    std::string query;
    char separator = '?';

    for ( const auto & it : query_parts )
    {
        query += separator + it.first + '=' +
            mf::utils::url::get_parameter::Encode(it.second);
        separator = '&';
    }

    const std::string url = url_path + query;

    return url;
}

std::string TokenlessApiBaseStatic::MakePost(
        const std::string & /*url_path*/,
        const std::map<std::string, std::string> & query_parts
    ) const
{
    std::string query;
    std::string separator;

    for ( const auto & it : query_parts )
    {
        query += separator + it.first + '=' +
            mf::utils::url::post_parameter::Encode(it.second);
        separator = '&';
    }

    return query;
}

}  // namespace detail
}  // namespace api
}  // namespace mf
