/**
 * @file url_encode.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "url_encode.hpp"

#include <cstdio>

#include <string>
#include <sstream>
#include <iomanip>

#include "boost/optional.hpp"
#include "boost/lexical_cast.hpp"

namespace {
    bool IsUnreserved(char ch)
    {
        if (
                ( 'A' <= ch && ch <= 'Z' ) ||   // character is between A and Z
                ( 'a' <= ch && ch <= 'z' ) ||   // character is between a and z
                ( '0' <= ch && ch <= '9' ) ||   // charcater is between 0 and 9
                ( '-' == ch ) ||                // character is - or
                ( '_' == ch ) ||                // character is _ or
                ( '.' == ch ) ||                // character is . or
                ( '~' == ch ) )                 // character is ~
            return false;
        else
            return true;
    }

std::string UrlEncodeBase(const std::string & unencoded_str, bool plus_encode)
{
    char output[3] = {0};

    std::string encoded_str;
    encoded_str.reserve(unencoded_str.size()*3);
    for ( std::string::const_iterator
            it = unencoded_str.begin(), end = unencoded_str.end();
            it != end; ++it )
    {
        if ( ! IsUnreserved( *it ) )
        {
            encoded_str += *it;
        }
        else if ( plus_encode && *it == ' ' )
        {
            encoded_str += "+";
        }
        else
        {
#if defined(_WIN32) && defined(_MSC_VER)
            sprintf_s(
                output, sizeof(output),
                "%02X", static_cast<unsigned char>(*it));
#else
            snprintf(
                output, sizeof(output),
                "%02X", static_cast<unsigned char>(*it));
#endif
            encoded_str += "%";
            output[2] = '\0';
            encoded_str += output;
        }
    }
    return encoded_str;
}
boost::optional<std::string> UrlUnencodeBase(
        const std::string & encoded_str,
        bool plus_encode
    )
{
    std::string unencoded_str;
    char buf[3] = {0};

    int unencoded_size = 0;
    for (
        std::string::const_iterator it = encoded_str.begin(),
        end = encoded_str.end();
        it != end;
        ++it
    )
    {
        if ( *it == '%' )
        {
            // Doing it == end compares now means we don't have to do them
            // later.
            ++it;
            if ( it == end )
                return boost::optional<std::string>();  // Fail
            ++it;
            if ( it == end )
                return boost::optional<std::string>();  // Fail

            ++unencoded_size;
        }
        else
        {
            ++unencoded_size;
        }
    }

    unencoded_str.reserve(unencoded_size);

    for ( std::string::const_iterator
        it = encoded_str.begin(), end = encoded_str.end();
        it != end; ++it )
    {
        if ( *it == '%' )
        {
            ++it; buf[0] = *it;
            ++it; buf[1] = *it;

            short int ch; // NOLINT "%hx" is short
            if (sscanf(buf, "%hx", &ch) == 1)
                unencoded_str += static_cast<char>(ch);
            else
                return boost::optional<std::string>();  // Fail
        }
        else if ( plus_encode && *it == '+' )
        {
            unencoded_str += ' ';
        }
        else
        {
            unencoded_str += *it;
        }
    }

    return unencoded_str;
}
}  // namespace

std::string mf::utils::UrlEncode(const std::string & unencoded_str)
{
    return UrlEncodeBase(unencoded_str, false);
}

std::string mf::utils::UrlPostEncode(const std::string & unencoded_str)
{
    return UrlEncodeBase(unencoded_str, true);
}

boost::optional<std::string> mf::utils::UrlUnencode(
        const std::string & encoded_str
    )
{
    return UrlUnencodeBase(encoded_str, false);
}

boost::optional<std::string> mf::utils::UrlPostUnencode(
        const std::string & encoded_str
    )
{
    return UrlUnencodeBase(encoded_str, true);
}
