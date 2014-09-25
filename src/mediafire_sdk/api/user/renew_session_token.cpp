/**
 * @file renew_session_token.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "renew_session_token.hpp"

#include <string>

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/url_encode.hpp"

#include "boost/property_tree/json_parser.hpp"

// #define OUTPUT_DEBUG

namespace v0 = mf::api::user::renew_session_token::v0;

v0::Request::Request(
        const std::string & session_token
    ) :
    session_token_(session_token)
{
}

std::string v0::Request::Url(std::string hostname) const
{
    std::string url;

    url = "https://" + hostname + "/api/user/renew_session_token.php";

    url += "?session_token=" +
        mf::utils::url::get_parameter::Encode(session_token_);

    url += "&response_format=json";

    return url;
}

void v0::Request::HandleContent(
        const std::string & url,
        const mf::http::Headers & headers,
        const std::string & content)
{
    assert( callback_ );
    if ( ! callback_ )
        return;

    ResponseType response;
    response.InitializeWithContent(url, "", headers, content);

#   ifdef OUTPUT_DEBUG // Debug code
    std::cout << "Got content:\n" << content << std::endl;

    std::ostringstream ss;
    boost::property_tree::write_json( ss, response.pt );
    std::cout << "Got JSON:\n" << ss.str() << std::endl;
#   endif

    if ( ! response.error_code )
    {
        if ( ! GetIfExists( response.pt, "response.session_token",
                    &response.session_token ) )
        {
            response.error_code = make_error_code(
                    api::api_code::ContentInvalidData );
            response.error_string = "missing session token";
        }
    }

    callback_(response);
}

void v0::Request::HandleError(
        const std::string & url,
        std::error_code ec,
        const std::string & error_string
    )
{
    if ( ! callback_ )
        return;

    ResponseType response;
    response.InitializeWithError(url, "", ec, error_string);
    callback_(response);
}

