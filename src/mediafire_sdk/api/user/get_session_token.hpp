/**
 * @file get_session_token.hpp
 * @author Herbert Jones
 * @brief API user/get_session_token
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include "get_session_token/v0.hpp"
#include "get_session_token/v1_3.hpp"

namespace mf {
namespace api {
/** API action path "user" */
namespace user {
/** API action "user/get_session_token" */
namespace get_session_token {

// Default version
using namespace v1_3;  // NOLINT

}  // namespace get_session_token
}  // namespace user
}  // namespace api
}  // namespace mf
