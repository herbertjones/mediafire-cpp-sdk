/**
 * @file connection_state.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "connection_state.hpp"

#include "mediafire_sdk/utils/variant.hpp"

using mf::utils::Match;

namespace mf
{
namespace api
{
namespace connection_state
{

bool operator==(const Uninitialized & /*lhs*/, const Uninitialized & /*rhs*/)
{
    return true;
}

bool operator==(const Unconnected & lhs, const Unconnected & rhs)
{
    // Only the parts we care about determine if the session token response is
    // different than another.

    return std::tie(lhs.error_code) == std::tie(rhs.error_code);
}

bool operator==(const Connected & /*lhs*/, const Connected & /*rhs*/)
{
    return true;
}

}  // namespace connection_state

std::ostream & operator<<(std::ostream & out, const ConnectionState & state)
{
    out << Match(
            state,
            [](const connection_state::Uninitialized &) -> std::string
            {
                return "Uninitialized{}";
            },
            [](const connection_state::Unconnected & unconnected) -> std::string
            {
                std::ostringstream ss;
                ss << "Unconnected{" << unconnected.error_code.message() << "}";
                return ss.str();
            },
            [](const connection_state::Connected &) -> std::string
            {
                return "Connected{}";
            });

    return out;
}

}  // namespace api
}  // namespace mf
