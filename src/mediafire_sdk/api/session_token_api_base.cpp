/**
 * @file session_token_api_base.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "session_token_api_base.hpp"

#include <map>
#include <string>
#include <utility>

#include "mediafire_sdk/utils/md5_hasher.hpp"
#include "mediafire_sdk/utils/url_encode.hpp"

namespace api = mf::api;

void api::detail::SessionTokenApiBaseStatic::SetSessionToken(
        std::string session_token,
        std::string time,
        int secret_key
    )
{
    session_token_ = session_token;
    time_ = time;
    secret_key_ = secret_key;
}

std::string api::detail::SessionTokenApiBaseStatic::MakePathAndQuery(
        const std::string & url_path,
        const std::map<std::string, std::string> & query_parts
    ) const
{
    if (GetRequestMethod() == RequestMethod::Post)
    {
        std::string query;
        char separator = '?';

        for ( const auto & it : query_parts )
        {
            query += separator + it.first + '='
                + mf::utils::UrlEncode(it.second);
            separator = '&';
        }

        return url_path + query;
    }
    else
    {
        /**
         * The API doesn't look at the raw path and query passed to it, but
         * rather decodes it first and re-assembles it.  Due to this we must
         * build a secondary query and path, but unencoded, to build the
         * signature against.
         */
        std::string query = "?session_token=" + session_token_;

        std::string unencoded_query = query;
        for ( const auto & it : query_parts )
        {
            query += '&' + it.first + '=' + mf::utils::UrlEncode(it.second);
            unencoded_query += '&' + it.first + '=' + it.second;
        }

        const std::string unencoded_url = url_path + unencoded_query;
        const std::string sig = GetSignature(unencoded_url);
        const std::string url = url_path + query + "&signature=" + sig;

        return url;
    }
}

std::string api::detail::SessionTokenApiBaseStatic::MakePost(
        const std::string & url_path,
        const std::map<std::string, std::string> & query_parts
    ) const
{
    /**
     * The API doesn't look at the raw path and query passed to it, but rather
     * decodes it first and re-assembles it.  Due to this we must build a
     * secondary query and path, but unencoded, to build the signature against.
     */
    std::string query = "session_token=" + session_token_;

    for ( const auto & it : query_parts )
    {
        query += '&' + it.first + '=' + mf::utils::UrlEncode(it.second);
    }

    const std::string signature_full_path = url_path + '?' + query;
    const std::string sig = GetSignature(signature_full_path);
    const std::string post_data = query + "&signature=" + sig;

    return post_data;
}

std::string api::detail::SessionTokenApiBaseStatic::GetSignature(
        const std::string & path_and_query
    ) const
{
    std::ostringstream ss;
    ss << ( secret_key_ % 256 );
    ss << time_;
    ss << path_and_query;

    return mf::utils::HashMd5(ss.str());
}
