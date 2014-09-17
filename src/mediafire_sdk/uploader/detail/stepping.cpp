/**
 * @file stepping.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "stepping.hpp"

namespace mf {
namespace uploader {
namespace detail {

const int MaxStepping = 7;

uint64_t SteppingMinFileSize(
        const uint32_t stepping
    )
{
    if (stepping == 0)
        return 0;

    // Marches table at:
    // http://www.mediafire.com/view/hifkclt4y3tn83l/resumable_uploads.odt
    return uint64_t(MB(4)) << ((stepping-1)*2);
}

uint32_t ThresholdStepping(
        const uint64_t filesize
    )
{
    uint32_t stepping = 0;

    uint64_t limit = SteppingMinFileSize(stepping+1);
    while(filesize >= limit && stepping < MaxStepping)
    {
        ++stepping;
        limit = SteppingMinFileSize(stepping+1);
    }

    return stepping;
}

uint64_t ChunkMaxSize(
        const uint64_t filesize
    )
{
    // 0 means upload the whole file ( which is less than 4 MB )
    const auto threshold_stepping = ThresholdStepping(filesize);
    auto unit_size = uint64_t(0);
    if (threshold_stepping == 0)
        unit_size = 1024*1024*4;
    else
        unit_size = 1024 * 1024 * (uint64_t(1) << (threshold_stepping-1));
    return unit_size;
}

// Uses idomatic C++, [begin, end)
std::vector<std::pair<uint64_t,uint64_t>> ChunkRanges(
        const uint64_t filesize
    )
{
    const auto chunk_size = ChunkMaxSize(filesize);
    auto ranges = std::vector<std::pair<uint64_t,uint64_t>>();
    auto current_end = uint64_t(0);
    auto current_chunk = uint64_t(0);

    while (current_end < filesize)
    {
        const auto current_start = current_end;

        current_end = std::min(
            chunk_size * (current_chunk+1),
            filesize );

        ranges.emplace_back(current_start, current_end);

        ++current_chunk;
    }

    return ranges;
}

}  // namespace detail
}  // namespace uploader
}  // namespace mf
