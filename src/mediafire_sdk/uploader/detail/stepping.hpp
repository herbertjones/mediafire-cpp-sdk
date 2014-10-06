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

template <uint64_t N>
struct MB_helper : std::integral_constant<uint64_t, N * 1024 * 1024> {};
template <uint64_t N>
struct GB_helper : std::integral_constant<uint64_t, MB_helper<N>::value * 1024> {};
template <uint64_t N>
struct TB_helper : std::integral_constant<uint64_t, GB_helper<N>::value * 1024> {};

#define MB(x) MB_helper<x>::value
#define GB(x) GB_helper<x>::value
#define TB(x) TB_helper<x>::value

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
