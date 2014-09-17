/**
 * @file sha1_hasher.hpp
 * @author Herbert Jones
 * @brief Convenient hashing algorithm wrappers.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once
#include <string>

#include "openssl/sha.h"
#include "types.hpp"

namespace mf {
namespace utils {

/**
 * @brief Hash a single string. Convenience function.
 *
 * @param[in] to_hash String to hash.
 *
 * @return SHA1 hash of input string.
 */
std::string HashSha1(std::string to_hash);

/**
 * @class Sha1Hasher
 * @brief Create a sha1 hash.
 */
class Sha1Hasher
{
public:
    Sha1Hasher();

    /**
     * @brief Add data to hasher.
     *
     * @param[in] size  The size of the buffer.
     * @param[in] bytes The buffer with the data to hash.
     */
    void Update(mf::FileSize size, void const * const bytes);

    /**
     * @brief Retrieve the hash digest as a hex encoded string.
     *
     * @return The hash as a hex encoded std::string.
     *
     * Calling this will reset the hash.
     */
    std::string Digest();

protected:
    /** Handle to hasher. */
    SHA_CTX hasher_;
};


}  // namespace utils
}  // namespace mf
