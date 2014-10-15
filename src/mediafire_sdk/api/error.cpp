/**
 * @file error.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "error.hpp"

bool mf::api::IsInvalidSessionTokenError(std::error_code error)
{
    return error == result_code::SessionTokenInvalid
        || error == result_code::SignatureInvalid;
}

bool mf::api::IsInvalidCredentialsError(std::error_code error)
{
    return error == result_code::CredentialsInvalid
        || error == result_code::ParametersInvalid
        || error == result_code::FacebookAuthenticationFailure;
}
