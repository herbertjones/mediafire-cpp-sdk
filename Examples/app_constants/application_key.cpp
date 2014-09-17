/**
 * @file application_key.cpp
 * @author Herbert Jones
 *
 * Copyright 2014 Mediafire
 */
#include "application_key.hpp"

#include <string>

#include <cstring>

#include "utils/sha1_hasher.hpp"

namespace ac = app_constants;

/** App id matching API key below. */
char const * const ac::kAppId = "EXAMPLE";

namespace {
char const * const kAppKey = "EXAMPLE";
}  // namespace

std::string ac::BuildSignature(
        const std::string & token
    )
{
    // Build signature
    utils::Sha1Hasher hasher;
    hasher.Update( token.size(), token.data() );
    hasher.Update( std::strlen(kAppId), kAppId );
    hasher.Update( std::strlen(kAppKey), kAppKey );
    return hasher.Digest();
}

