/**
 * @file app_constants.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "app_constants.hpp"

#include <string>

#include <cstring>

#include "mediafire_sdk/utils/sha1_hasher.hpp"

#define STATIC_STRLEN(s) (sizeof(s)/sizeof(s[0])-1)

namespace app_constants {

/**
 * App id matching API key below.
 *
 * We allow overriding the value or the variable altogether.
 */
#ifndef MFAPI_OVERRIDE_APP_ID
#   ifdef MFAPI_APP_ID
        char const * const kAppId = MFAPI_APP_ID;
#   else
#       error "You must supply an APP ID for the SDK to function."
#   endif
#endif

#ifndef MFAPI_OVERRIDE_BUILD_SIGNATURE
std::string BuildSignature(
        const std::string & token
    )
{
#ifdef MFAPI_APP_KEY
    const char kAppKey[] = MFAPI_APP_KEY;
#else
    const char kAppKey[] = "";
#endif

    static const bool has_app_id = (app_constants::kAppId != nullptr
        && std::strlen(app_constants::kAppId) > 0);

    // Build signature
    mf::utils::Sha1Hasher hasher;
    hasher.Update( token.size(), token.data() );

    if (has_app_id)
    {
        hasher.Update( std::strlen(kAppId), kAppId );
    }

    // Static expression should get optimized to always yes or no...
    if (STATIC_STRLEN(kAppKey) > 0)
    {
        hasher.Update( std::strlen(kAppKey), kAppKey );
    }

    return hasher.Digest();
}
#endif

}  // namespace app_constants
