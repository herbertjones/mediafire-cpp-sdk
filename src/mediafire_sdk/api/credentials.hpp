/**
 * @file credentials.hpp
 * @author Herbert Jones
 * @brief Credentials types
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>

#include "boost/variant/variant.hpp"

namespace mf {
namespace api {
namespace credentials {

/**
 * @struct Email
 * @brief Wrapper for session login via email and password.
 *
 * Email is part of the Credentials variant wrapper.
 */
struct Email
{
    /** Email associated with account. */
    std::string email;

    /** Password to log into the account. */
    std::string password;
};

/**
 * @struct Ekey
 * @brief Wrapper for session login via ekey and password.
 *
 * The ekey (pronounced "e key") is an identifier that uniquely identifies a
 * user regardless of email address changes and should be the preferred way to
 * log in.
 */
struct Ekey
{
    /** Ekey returned by user/get_info and user/get_session_token. */
    std::string ekey;

    /** User's password. */
    std::string password;
};

/**
 * @struct Facebook
 * @brief Wrapper for session login via facebook token.
 *
 * Facebook is part of the Credentials variant wrapper.
 */
struct Facebook
{
    /** The facebook access token. */
    std::string fb_access_token;
};

}  // namespace credentials

/**
 * @typedef Credentials
 * @brief Credentials variant encompassing Email and
 * Facebook.
 */
typedef boost::variant
    < credentials::Email
    , credentials::Ekey
    , credentials::Facebook
    > Credentials;

namespace credentials {

/**
 * @brief Compare two Email.
 *
 * @param[in] lhs Left side
 * @param[in] rhs Right side
 *
 * @return True if equal.
 */
bool operator==(const Email& lhs, const Email& rhs);

/**
 * @brief Compare two Ekeys.
 *
 * @param[in] lhs Left side
 * @param[in] rhs Right side
 *
 * @return True if equal.
 */
bool operator==(const Ekey& lhs, const Ekey& rhs);

/**
 * @brief Compare two Facebook.
 *
 * @param[in] lhs Left side
 * @param[in] rhs Right side
 *
 * @return True if equal.
 */
bool operator==(const Facebook& lhs, const Facebook& rhs);

}  // namespace credentials

/**
 * @brief Create a hash of the credentials to quickly verify if credentials have
 * changed.
 *
 * @param[in] credentials Credentials
 *
 * @return Numeric hash value.
 */
std::size_t CredentialsHash(Credentials credentials);

}  // namespace api
}  // namespace mf
