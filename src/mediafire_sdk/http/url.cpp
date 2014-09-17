/**
 * @file url.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "url.hpp"

#include <algorithm>
#include <iterator>
#include <iostream>
#include <string>

namespace hl = mf::http;

namespace {
    const std::string prot_sep(":");
    const std::string host_begin("//");
    const std::string prot_end("://");
    const std::string path_sep("/");
    const std::string login_sep("@");
    const std::string pass_sep(":");
    const std::string port_sep(":");
    const std::string query_sep("?");
    const std::string hash_sep("#");
}  // namespace

typedef std::string::const_iterator ci;

hl::Url::Url(const std::string & url) : url_(url)
{
    const ci url_begin = url_.begin();
    const ci url_end = url_.end();

    path_ = "/";
    full_path_ = "/";

    // Get protocol
    ci it = std::search( url_begin,
            url_end, prot_end.begin(), prot_end.end() );

    if ( it == url_end )
        throw InvalidUrl("Invalid scheme separator");

    scheme_.reserve( std::distance( url_begin, it ) );
    std::copy( url_begin, it, std::back_inserter(scheme_) );

    std::advance( it, std::distance( prot_end.begin(), prot_end.end() )
            );

    const ci chunk_begin = it;
    const ci chunk_end = std::search(
            chunk_begin, url_end, path_sep.begin(), path_sep.end() );

    it = std::search( chunk_begin, chunk_end, login_sep.begin(),
            login_sep.end() );

    ci host_begin = chunk_begin;

    if ( it != chunk_end )
    {
        host_begin = it;
        std::advance( host_begin, std::distance( login_sep.begin(),
                    login_sep.end() ) );

        // Get login data
        const ci login_begin = chunk_begin;
        const ci login_end = it;

        it = std::search( login_begin, login_end, pass_sep.begin(),
                pass_sep.end() );
        if ( it != login_end )
        {
            user_.reserve( std::distance( login_begin, it ) );
            std::copy( login_begin, it, std::back_inserter(user_) );
            std::advance(
                    it,
                    std::distance(
                        pass_sep.begin(),
                        pass_sep.end() )
                );

            pass_.reserve( std::distance( it, login_end ) );
            std::copy( it, login_end, std::back_inserter(pass_) );
        }
    }

    it = std::search( host_begin, chunk_end, port_sep.begin(),
            port_sep.end() );

    host_.reserve( std::distance( host_begin, it ) );
    std::copy( host_begin, it, std::back_inserter(host_) );

    if ( it != chunk_end )
    {
        // Has port
        std::advance( it, std::distance( port_sep.begin(),
                    port_sep.end() ) );
        port_.reserve( std::distance( host_begin, it ) );
        std::copy( it, chunk_end, std::back_inserter(port_) );
    }

    if (chunk_end != url_end)
        ParsePath(chunk_end);

    // else nothing else to do.
}

void hl::Url::ParsePath(std::string::const_iterator path_start)
{
    const ci url_end = url_.end();

    path_ = "/";
    full_path_ = "/";

    // Find query
    ci query = std::search( path_start, url_end, query_sep.begin(),
            query_sep.end() );
    ci hash, path_end = url_end;

    if ( query != url_end )
    {
        // Has query string
        hash = std::search( query, url_end, hash_sep.begin(),
                hash_sep.end() );
        path_end = query;

        std::advance( query, std::distance( query_sep.begin(),
                    query_sep.end() ));

        query_.reserve( std::distance( query, hash ) );
        std::copy( query, hash, std::back_inserter(query_) );
    }
    else
    {
        hash = std::search( path_start, url_end, hash_sep.begin(),
                hash_sep.end() );

        path_end = hash;
    }

    if (hash != url_end)
    {
        std::advance( hash, std::distance( hash_sep.begin(),
                    hash_sep.end() ));

        fragment_.reserve( std::distance( hash, url_end ) );
        std::copy( hash, url_end, std::back_inserter(fragment_) );
    }


    path_.clear();
    path_.reserve( std::distance( path_start, path_end ) );
    std::copy( path_start, path_end, std::back_inserter(path_) );

    full_path_.clear();
    full_path_.reserve( std::distance( path_start, url_end ) );
    std::copy( path_start, url_end, std::back_inserter(full_path_) );
}

hl::Url hl::Url::FromRedirect(const std::string & redirect) const
{
    // If protocol string exists, then we can return a new Url
    ci it = std::search( redirect.begin(),
            redirect.end(), prot_end.begin(), prot_end.end() );

    if ( it != redirect.end() )
    {
        return Url(redirect);
    }
    else
    {
        // Returned url should always start with scheme
        std::string url = scheme();

        // Check for host
        if ( redirect.compare(0, host_begin.size(), host_begin) == 0 )
        {
            // Protocol is missing but URL has a host- just append to scheme
            url += prot_sep + redirect;
        }
        else
        {
            // URL has no host- append redirect to existing host
            url += prot_end;
            if ( ! user_.empty() || ! pass_.empty() )
                url += user_ + pass_sep + pass_ + login_sep;

            url += host_;
            if ( ! port_.empty() )
                url += port_sep + port_;

            url += redirect;
        }

        return Url(url);
    }
}


