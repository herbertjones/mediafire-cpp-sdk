/**
 * @file app_constants.hpp
 * @author Herbert Jones
 * @brief Internal constants available for override.
 * @copyright Copyright 2014 Mediafire
 *
 * The constants here can be overridden by your application, depending on how
 * you want to connect to the MediaFire API.
 */
#pragma once

#include <string>

namespace app_constants {

/**
 * @brief The application id. It is a string that contains the app id number.
 *
 * This will be NULL if not overridden.  It is only needed in certain
 * circumstances..
 */
extern char const * const kAppId;

/**
 * @brief Build the get_session_token signature.
 *
 * @param[in] token One of the following, joined together with no spaces if
 * possible: email and password, facebook access token, twitter oauth token and
 * secret.
 *
 * @return The signature to use in the get_session_token API request.
 */
std::string BuildSignature(
        const std::string & token
    );

}  // namespace app_constants
