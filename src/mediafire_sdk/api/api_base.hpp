/**
 * @file api_base.hpp
 * @author Herbert Jones
 * @brief Base class for API request types.
 *
 * @copyright Copyright 2014 Mediafire
 *
 * See api_interface.hpp for full example on how to create an API request type.
 */
#pragma once

#include <functional>
#include <map>
#include <string>

#include "mediafire_sdk/http/headers.hpp"
#include "mediafire_sdk/http/http_request.hpp"
#include "mediafire_sdk/api/response_base.hpp"
#include "mediafire_sdk/utils/string.hpp"

#include "boost/property_tree/json_parser.hpp"

namespace mf {
namespace api {

/** The intended target HTTP method. */
enum class RequestMethod {
    /** The request method is "GET" */
    Get,
    /** The request method is "POST" */
    Post
};

namespace detail {

/**
 * @class ApiBase
 * @brief Templated base class for API operations.
 *
 * Contains base functionality for API calls which are custom to each API type.
 */
template<typename DataType>
class ApiBase
{
public:
    /** Requester/SessionMaintainer expected class. */
    typedef DataType ResponseType;

    /** Requester/SessionMaintainer expected typedef. */
    typedef std::function<
        void(
                const ResponseType & data
            )> CallbackType;

    /** Requester/SessionMaintainer expected typedef. */
    virtual void SetCallback( CallbackType callback_function )
    {
        callback_ = callback_function;
    };

    /** Requester expected method. */
    virtual void HandleContent(
            const std::string & url,
            const mf::http::Headers & headers,
            const std::string & content)
    {
        assert( callback_ );

        ResponseType response;
        response.InitializeWithContent(url, api_data_type_debug_, headers, content);

#       ifdef OUTPUT_DEBUG // Debug code
        std::cout << "Got content:\n" << content << std::endl;

        std::wostringstream ss;
        boost::property_tree::write_json( ss, response.pt );
        std::cout << "Got JSON:\n" << mf::utils::wide_to_bytes(ss.str()) << std::endl;
#       endif

        if ( ! response.error_code )
        {
            ParseResponse(&response);

            // Record error if one ocurred during parsing
            if ( response.error_code )
            {
                response.debug += "Error: " + response.error_code.message()
                    + "\n";
                if (response.error_string)
                {
                    response.debug += "Error text: " + *response.error_string
                        + "\n";
                }
            }
        }

        callback_(response);
    }

    /** Requester expected method. */
    virtual void HandleError(
            const std::string & url,
            std::error_code ec,
            const std::string & error_string
        )
    {
#       ifdef OUTPUT_DEBUG // Debug code
        std::cout << "Got error:\n" << ec.message() << std::endl;
        std::cout << "Got error description:\n" << error_string << std::endl;
#       endif

        ResponseType response;
        response.InitializeWithError(url, api_data_type_debug_, ec, error_string);

        if ( callback_ )
            callback_(response);
    }

    /**
     * @brief RequestMethod::Post if request has POST methods to pass.
     *
     * @return Get or Post depending on method used
     */
    virtual api::RequestMethod GetRequestMethod() const = 0;

protected:
    CallbackType callback_;
    std::string api_data_type_debug_;

    /**
     * @brief Pure virtual function so implementer only needs to extract data
     * from already parsed JSON.
     *
     * Classes that wish to use this base must use the property tree in the
     * ResponseType to extract data from the API JSON, or set the error fields
     * in the ResponseType if a failure is detected.
     *
     * @param[in,out] response Pointer to ResponseType with parsed JSON.
     */
    virtual void ParseResponse(ResponseType * response) = 0;

    /**
     * @brief Convenience function for creating a session token based URL.
     *
     * @param[out] path The path of the API URL. Example:
     *             /api/user/get_action_token.php
     * @param[in,out] query_parts Map with url query key pairs. If query
     *                contains "type=upload", then:
     *                query_parts["type"] == "upload"
     */
    virtual void BuildUrl(
            std::string * path,
            std::map<std::string, std::string> * query_parts
        ) const = 0;

    /**
     * @brief Convenience function for setting the error fields of the
     *        Response.
     *
     * @param[in,out] response Response
     * @param[in] error_id Error code id
     * @param[in] error_str Error message
     */
    template<typename Response, typename ErrorId>
    void SetError(
            Response * response,
            ErrorId error_id,
            std::string error_str
        )
    {
        response->error_code = make_error_code( error_id );
        response->error_string = error_str;
    }
};

}  // namespace detail
}  // namespace api
}  // namespace mf
