/**
 * @file session_state.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "session_state.hpp"

namespace mf {
namespace api {
namespace session_state {

bool operator==(const Uninitialized& /*lhs*/, const Uninitialized& /*rhs*/)
{
    return true;
}

bool operator==(const Initialized& /*lhs*/, const Initialized& /*rhs*/)
{
    return true;
}

bool operator==(const CredentialsFailure& lhs, const CredentialsFailure& rhs)
{
    // Only the parts we care about determine if the session token response is
    // different than another.

    return std::tie(lhs.pkey, lhs.error_code)
        == std::tie(rhs.pkey, rhs.error_code);
}

bool operator==(const Running& lhs, const Running& rhs)
{
    const api::user::get_session_token::Response & l =
        lhs.session_token_response;

    const api::user::get_session_token::Response & r =
        rhs.session_token_response;

    // Only the parts we care about determine if the session token response is
    // different than another.

    return std::tie(l.session_token, l.pkey)
        == std::tie(r.session_token, r.pkey);
}

}  // namespace session_state
}  // namespace api
}  // namespace mf
