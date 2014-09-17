/**
 * @file sha1_hasher.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "sha1_hasher.hpp"

#include <cstdio>

#include <string>

mf::utils::Sha1Hasher::Sha1Hasher()
{
    SHA1_Init(&hasher_);
}

// SHA1_* functions are deprecated on OS X 10.7+. It works for now but I don't
// want to be flooded with warnings, so I just left one in. -ZCM
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

void mf::utils::Sha1Hasher::Update(mf::FileSize size, void const * const bytes)
{
    SHA1_Update(&hasher_, bytes, static_cast<std::size_t>(size));
}

std::string mf::utils::Sha1Hasher::Digest()
{
    unsigned char digest[ SHA_DIGEST_LENGTH ];

    SHA1_Final(digest, &hasher_);

    std::string hash;
    hash.reserve(SHA_DIGEST_LENGTH * 2 + 1);

    for (unsigned int i = 0; i < SHA_DIGEST_LENGTH ; i++)
    {
        char output[3];
#if defined(_WIN32) && defined(_MSC_VER)
        sprintf_s(output, sizeof(output), "%02x", digest[i]);
#else
        snprintf(output, sizeof(output), "%02x", digest[i]);
#endif
        output[2] = '\0';
        hash += output;
    }

    // Reset context for next potential chunk.
    SHA1_Init(&hasher_);

    return hash;
}

std::string mf::utils::HashSha1(std::string str)
{
    mf::utils::Sha1Hasher hasher;
    hasher.Update(str.size(), str.data());
    return hasher.Digest();
}

#pragma clang diagnostic pop
