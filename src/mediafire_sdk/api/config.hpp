/**
 * @file config.hpp
 * @author Herbert Jones
 * @brief Configuration types
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

namespace mf {
namespace api {

/** Option to prevent SessionMaintainer from starting API HttpRequest.
 * Used if additional options must be set on HttpRequest object. */
enum class RequestStarted {
    /** Do not start the HttpRequest immediately. */
    No,
    /** Start the HttpRequest before passing back to caller. */
    Yes,
};

}  // namespace api
}  // namespace mf
