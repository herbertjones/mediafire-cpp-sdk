/**
 * @file stepping.hpp
 * @author Herbert Jones
 * @brief Helper functions for chunk stepping
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <cstdint>
#include <vector>

namespace mf {
namespace uploader {
namespace detail {

extern const int MaxStepping;

constexpr uint64_t MB(const uint64_t x)
{
    return x*1024*1024;
}
constexpr uint64_t GB(const uint64_t x)
{
    return MB(x)*1024;
}
constexpr uint64_t TB(const uint64_t x)
{
    return GB(x)*1024;
}

uint64_t SteppingMinFileSize(
        const uint32_t stepping
    );
uint32_t ThresholdStepping(
        const uint64_t filesize
    );
uint64_t ChunkMaxSize(
        const uint64_t filesize
    );

// Uses idomatic C++, [begin, end)
std::vector<std::pair<uint64_t,uint64_t>> ChunkRanges(
        const uint64_t filesize
    );

}  // namespace detail
}  // namespace uploader
}  // namespace mf
