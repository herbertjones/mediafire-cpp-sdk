/**
 * @file sha256_hasher.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "sha256_hasher.hpp"

#include <cstdio>

#include <string>

mf::utils::Sha256Hasher::Sha256Hasher()
{
    SHA256_Init(&hasher_);
}

// SHA256_* functions are deprecated on OS X 10.7+. It works for now but I don't
// want to be flooded with warnings, so I just left one in. -ZCM
#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

void mf::utils::Sha256Hasher::Update(mf::FileSize size, void const * const bytes)
{
    SHA256_Update(&hasher_, bytes, static_cast<std::size_t>(size));
}

std::string mf::utils::Sha256Hasher::Digest()
{
    unsigned char digest[ SHA256_DIGEST_LENGTH ];

    SHA256_Final(digest, &hasher_);

    std::string hash;
    hash.reserve(SHA256_DIGEST_LENGTH * 2 + 1);

    for (unsigned int i = 0; i < SHA256_DIGEST_LENGTH ; i++)
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
    SHA256_Init(&hasher_);

    return hash;
}

std::string mf::utils::HashSha256(std::string str)
{
    mf::utils::Sha256Hasher hasher;
    hasher.Update(str.size(), str.data());
    return hasher.Digest();
}

#if defined(__clang__)
#  pragma clang diagnostic pop
#endif
