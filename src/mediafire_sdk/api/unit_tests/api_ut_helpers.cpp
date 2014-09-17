/**
 * @file api_ut_helpers.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#define WIN32_LEAN_AND_MEAN
#include "api_ut_helpers.hpp"

#include <string>

#include "mediafire_sdk/utils/string.hpp"

namespace api = mf::api;

std::string api::ut::ToString(
        const boost::property_tree::wptree & pt
    )
{
    std::string response;
    {
        std::wostringstream ss;
        boost::property_tree::write_json( ss, pt );
        response = mf::utils::wide_to_bytes(ss.str());
    }
    return response;
}

boost::property_tree::wptree api::ut::MakeBaseApiContent(
        std::string action
    )
{
    // Example JSON:
    // {
    //     "response"
    //     {
    //         "action": "user\/get_session_token",
    //         "result": "Success",
    //     }
    // }

    boost::property_tree::wptree pt;
    pt.put( L"response.action", mf::utils::bytes_to_wide(action) );
    pt.put( L"response.result", L"Success" );
    return pt;
}

boost::property_tree::wptree api::ut::MakeErrorApiContent(
        std::string action,
        std::string api_error_text,
        int api_error_code
    )
{
    // Example JSON:
    // {
    //     "response":
    //     {
    //         "action": "user\/get_session_token",
    //         "pkey": "09b3b02524",
    //         "message": "The Credentials you entered are invalid",
    //         "error": "107",
    //         "result": "Error",
    //         "current_api_version": "2.14"
    //     }
    // }

    boost::property_tree::wptree pt;
    pt.put( L"response.action", mf::utils::bytes_to_wide(action) );
    pt.put( L"response.message", mf::utils::bytes_to_wide(api_error_text) );
    pt.put( L"response.error", api_error_code );
    pt.put( L"response.result", L"Error" );
    return pt;
}
