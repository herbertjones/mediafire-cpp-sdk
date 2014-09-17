/**
 * @file application_key.hpp
 * @author Herbert Jones
 * @brief Application id constants
 *
 * Copyright 2014 Mediafire
 *
 * The app_constants namespace should be unique to each application, as it
 * contains the application key.
 */
#pragma once

#include <string>

namespace app_constants {

/**
 * @brief The application id. It is a string that contains the app id number.
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
