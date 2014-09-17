/**
 * @file session_token_test.hpp
 * @author Herbert Jones
 * @brief Constants for session token testing
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <map>
#include <memory>
#include <string>

namespace mf {
namespace api {
namespace ut {

class Request;

typedef std::shared_ptr<Request> RequestPointer;
typedef std::function<void(RequestPointer)> PathHandler;
typedef std::map<std::string, PathHandler> PathHandlers;

enum HttpStatusCode
{
    ok                    = 200,
    created               = 201,
    accepted              = 202,
    no_content            = 204,
    multiple_choices      = 300,
    moved_permanently     = 301,
    moved_temporarily     = 302,
    not_modified          = 304,
    bad_request           = 400,
    unauthorized          = 401,
    forbidden             = 403,
    not_found             = 404,
    internal_server_error = 500,
    not_implemented       = 501,
    bad_gateway           = 502,
    service_unavailable   = 503,
};

}  // namespace ut
}  // namespace api
}  // namespace mf
