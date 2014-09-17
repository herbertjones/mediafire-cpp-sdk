/**
 * @file session_state.hpp
 * @author Herbert Jones
 * @brief Session states for session token maintainer.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <system_error>

#include "boost/variant/variant.hpp"
#include "boost/optional.hpp"

#include "mediafire_sdk/api/user/get_session_token.hpp"

namespace mf {
namespace api {
namespace session_state {

/** Initial uninitialized session state */
struct Uninitialized {};
/** Session initialized with credentials */
struct Initialized {};
/** Supplied credentials can not be used. */
struct CredentialsFailure
{
    /** Set if pkey was supplied by the server.  If different than previous pkey
     * value, the password has been changed. */
    boost::optional<std::string> pkey;

    /** Any error that occured in the process. */
    std::error_code error_code;

    /** Response data if more data required. */
    api::user::get_session_token::Response session_token_response;
};
/** Credentials used and tokens successfully acquired. */
struct Running
{
    /** Full response from the session token request that succeeded. */
    api::user::get_session_token::Response session_token_response;
};

/**
 * @brief Compare two Uninitialized.
 *
 * @param[in] lhs Left side
 * @param[in] rhs Right side
 *
 * @return True if equal.
 */
bool operator==(const Uninitialized& lhs, const Uninitialized& rhs);

/**
 * @brief Compare two Initialized.
 *
 * @param[in] lhs Left side
 * @param[in] rhs Right side
 *
 * @return True if equal.
 */
bool operator==(const Initialized& lhs, const Initialized& rhs);

/**
 * @brief Compare two CredentialsFailure.
 *
 * @param[in] lhs Left side
 * @param[in] rhs Right side
 *
 * @return True if equal.
 */
bool operator==(const CredentialsFailure& lhs, const CredentialsFailure& rhs);

/**
 * @brief Compare two Running.
 *
 * @param[in] lhs Left side
 * @param[in] rhs Right side
 *
 * @return True if equal.
 */
bool operator==(const Running& lhs, const Running& rhs);

}  // namespace session_state

/** Union like structure which holds all possible session states. */
typedef boost::variant
    < session_state::Uninitialized
    , session_state::Initialized
    , session_state::CredentialsFailure
    , session_state::Running
    > SessionState;

}  // namespace api
}  // namespace mf

