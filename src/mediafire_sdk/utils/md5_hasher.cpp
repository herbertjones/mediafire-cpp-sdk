/**
 * @file sha256_hasher.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "md5_hasher.hpp"

#include <cstdio>

#include <string>

mf::utils::Md5Hasher::Md5Hasher()
{
    MD5_Init(&hasher_);
}

// MD5_* functions are deprecated on OS X 10.7+. It works for now but I don't
// want to be flooded with warnings, so I just left one in. -ZCM
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

void mf::utils::Md5Hasher::Update(mf::FileSize size, void const * const bytes)
{
    MD5_Update(&hasher_, bytes, static_cast<std::size_t>(size));
}

std::string mf::utils::Md5Hasher::Digest()
{
    unsigned char digest[ MD5_DIGEST_LENGTH ];

    MD5_Final(digest, &hasher_);

    std::string hash;
    hash.reserve(MD5_DIGEST_LENGTH * 2 + 1);

    for (unsigned int i = 0; i < MD5_DIGEST_LENGTH ; i++)
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
    MD5_Init(&hasher_);

    return hash;
}

std::string mf::utils::HashMd5(std::string str)
{
    mf::utils::Md5Hasher hasher;
    hasher.Update(str.size(), str.data());
    return hasher.Digest();
}

#pragma clang diagnostic pop
