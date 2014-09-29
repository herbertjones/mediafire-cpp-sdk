/**
 * @file response_base.hpp
 * @author Herbert Jones
 * @brief Base class for the JSON API request return object.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>
#include <system_error>

#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"

#include "mediafire_sdk/http/headers.hpp"

namespace mf {
namespace api {

/**
 * @interface ResponseBase
 * @brief Base class for API Response objects.
 */
class ResponseBase
{
public:
    /** Any error that occured in the process. */
    std::error_code error_code;

    /** Set if error occurred and had extra description. */
    boost::optional<std::string> error_string;

    /** Set if API error occurred and had API error message. */
    boost::optional<std::string> api_error_string;

    /** The HTTP content. For debugging. */
    std::string plaintext;

    /**
     * Property tree extracted from JSON.
     *
     * @warning:  property_tree doesn't correctly parse unicode when using
     * std::string as a base.  Wstring used to be able to parse unicode
     * characters.
     */
    boost::property_tree::wptree pt;

    /** The url used to make the request. */
    std::string url;

    /** Useful debug information */
    std::string debug;

    /**
     * @brief Initializer when content was received from the server with no
     * errors.
     *
     * @param[in] request_url Request url
     * @param[in] debug Debug data to pass to debug member variable
     * @param[in] headers Http response headers
     * @param[in] content JSON encoded data from the remote server.
     *
     * The error_code will be set if an error occurred while parsing the
     * content.
     */
    void InitializeWithContent(
            const std::string & request_url,
            const std::string & debug,
            const mf::http::Headers & headers,
            const std::string & content
        );

    /**
     * @brief Initialized when HTTP request failed.
     *
     * @param[in] request_url Request url
     * @param[in] debug Debug data to pass to debug member variable
     * @param[in] ec Error code passed from the HTTP request object.
     * @param[in] error_str Error text passed from the HTTP request object.
     */
    void InitializeWithError(
            const std::string & request_url,
            const std::string & debug,
            std::error_code ec,
            const std::string & error_str
        );

    /**
     * @brief Default constructor. Don't use outside unit testing.
     */
    ResponseBase();

    virtual ~ResponseBase();
};

}  // namespace api
}  // namespace mf
