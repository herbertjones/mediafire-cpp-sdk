/**
 * @file connection_state.hpp
 * @author Herbert Jones
 * @brief Connection states for session token maintainer.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <system_error>

#include "boost/variant/variant.hpp"
#include "boost/optional.hpp"

#include "mediafire_sdk/api/user/get_session_token.hpp"

namespace mf
{
namespace api
{
namespace connection_state
{

// States
/** Initial uninitialized connection state */
struct Uninitialized
{
};
/** connection to remote server has been disrupted. */
struct Unconnected
{
    /** Any error that occurred in the process. */
    std::error_code error_code;
};
/** State when connection established with remote server. */
struct Connected
{
};

// Comparison operators
/**
 * @brief Compare two Uninitialized.
 *
 * @param[in] lhs Left side
 * @param[in] rhs Right side
 *
 * @return True if equal.
 */
bool operator==(const Uninitialized & lhs, const Uninitialized & rhs);

/**
 * @brief Compare two Unconnected.
 *
 * @param[in] lhs Left side
 * @param[in] rhs Right side
 *
 * @return True if equal.
 */
bool operator==(const Unconnected & lhs, const Unconnected & rhs);

/**
 * @brief Compare two Connected.
 *
 * @param[in] lhs Left side
 * @param[in] rhs Right side
 *
 * @return True if equal.
 */
bool operator==(const Connected & lhs, const Connected & rhs);

}  // namespace connection_state

/** Union like structure which holds all possible connection states. */
typedef boost::variant<connection_state::Uninitialized,
                       connection_state::Unconnected,
                       connection_state::Connected> ConnectionState;

std::ostream & operator<<(std::ostream & out, const ConnectionState & state);

}  // namespace api
}  // namespace mf
