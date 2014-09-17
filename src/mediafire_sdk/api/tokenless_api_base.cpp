/**
 * @file tokenless_api_base.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "tokenless_api_base.hpp"

#include <map>
#include <string>
#include <utility>

#include "mediafire_sdk/utils/url_encode.hpp"

namespace api = mf::api;

std::string api::detail::TokenlessApiBaseStatic::MakePathAndQuery(
        const std::string & url_path,
        const std::map<std::string, std::string> & query_parts
    ) const
{
    std::string query;
    char separator = '?';

    for ( const auto & it : query_parts )
    {
        query += separator + it.first + '=' + mf::utils::UrlEncode(it.second);
        separator = '&';
    }

    const std::string url = url_path + query;

    return url;
}

std::string api::detail::TokenlessApiBaseStatic::MakePost(
        const std::string & url_path,
        const std::map<std::string, std::string> & query_parts
    ) const
{
    std::string query;
    std::string separator;

    for ( const auto & it : query_parts )
    {
        query += separator + it.first + '=' + mf::utils::UrlEncode(it.second);
        separator = '&';
    }

    return query;
}

