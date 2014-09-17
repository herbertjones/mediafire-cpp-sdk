/**
 * @file connection_state.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "connection_state.hpp"

namespace mf {
namespace api {
namespace connection_state {

bool operator==(const Uninitialized& /*lhs*/, const Uninitialized& /*rhs*/)
{
    return true;
}

bool operator==(const Unconnected& lhs, const Unconnected& rhs)
{
    // Only the parts we care about determine if the session token response is
    // different than another.

    return std::tie(lhs.error_code)
        == std::tie(rhs.error_code);
}

bool operator==(const Connected& /*lhs*/, const Connected& /*rhs*/)
{
    return true;
}

}  // namespace connection_state
}  // namespace api
}  // namespace mf
