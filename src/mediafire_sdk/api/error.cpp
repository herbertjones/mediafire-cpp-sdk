/**
 * @file error.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "error.hpp"

namespace api = mf::api;

bool api::IsInvalidSessionTokenError(std::error_code error)
{
    return error == errc::SessionTokenInvalid
        || error == errc::SignatureInvalid;
}

