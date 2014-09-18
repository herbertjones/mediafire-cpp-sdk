/**
 * @file response_base.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "response_base.hpp"

#include <string>

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/utils/string.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace api = mf::api;

api::ResponseBase::ResponseBase()
{
}

api::ResponseBase::~ResponseBase()
{
}

void api::ResponseBase::InitializeWithContent(
        const std::string & request_url,
        const std::string & debug_passthru,
        const mf::http::Headers & headers,
        const std::string & content
    )
{
    bool parsing_success = false;

    url = request_url;
    plaintext = content;

    debug += "Url: " + url + "\n";
    debug += debug_passthru;
    debug += "Plaintext: " + plaintext + "\n";

    const std::wstring content_wstr = mf::utils::bytes_to_wide(content);

    std::wistringstream istr(content_wstr);

    try {
        boost::property_tree::read_json(
                istr,  // Input stream
                pt  // Output object
            );
        parsing_success = true;

        {
            std::wostringstream ss;
            boost::property_tree::write_json( ss, pt );
            debug += "Formatted JSON: ";
            debug += mf::utils::wide_to_bytes(ss.str());
            if (debug.back() != '\n')
                debug += "\n";
        }
    }
    catch ( boost::property_tree::json_parser_error & err )
    {
        error_code = make_error_code(
                api::api_code::ContentInvalidFormat );
        error_string = err.what();
    }

    // Check for errors. According to Rabie, any API error will have a non 200
    // HTTP response code.
    if ( headers.status_code != 200 )
    {
        // Use the HTTP error code.  Will be replaced if error can be parsed
        // below.
        error_code = std::error_code(headers.status_code,
            api::http_status_category());

        // Try to get the error from the remote API call.
        if ( parsing_success )
        {
            std::string api_message;
            if ( GetIfExists( pt, "response.message", &api_message ) )
            {
                error_string = api_message;
                api_error_string = api_message;
            }
            else
            {
                error_string =
                    "API returned bad HTTP status and no error message.";
            }

            int32_t api_code = 0;
            if ( GetIfExists( pt, "response.error", &api_code ) )
            {
                api_error_code = api_code;
                error_code = std::error_code(api_code, result_category());
            }
        }
        else
        {
            error_string = error_code.message();
        }
    }
    else if ( parsing_success )
    {
        // Yet I don't trust that.
        if ( ! PropertyHasValueCaseInsensitive(
                    pt, "response.result", "success" ) )
        {
            int32_t api_code = 0;
            if ( GetIfExists( pt, "response.error", &api_code ) )
            {
                api_error_code = api_code;
                error_code = std::error_code(api_code, result_category());
            }
            else
            {
                error_code = make_error_code( api::api_code::UnknownApiError );
            }

            std::string api_message;
            if ( GetIfExists( pt, "response.message", &api_message ) )
            {
                error_string = api_message;
                api_error_string = api_message;
            }
            else
            {
                error_string = "API returned bad result and no error message.";
            }
        }
    }

    if (error_code)
        debug += "Error: " + error_code.message() + "\n";
    if (error_string)
        debug += "Error text: " + *error_string + "\n";

#ifdef OUTPUT_DEBUG // Debug code
    std::cout << debug << std::endl;
#endif
}

void api::ResponseBase::InitializeWithError(
        const std::string & request_url,
        const std::string & debug_passthru,
        std::error_code ec,
        const std::string & error_str
    )
{
    url = request_url;
    error_code = ec;
    error_string = error_str;

    debug += "Url: " + url + "\n";
    debug += debug_passthru;
    debug += "Error: " + ec.message() + "\n";
    debug += "Error text: " + error_str + "\n";
}
