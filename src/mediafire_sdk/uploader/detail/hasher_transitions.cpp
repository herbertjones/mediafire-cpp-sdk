/**
 * @file hasher_transitions.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "hasher_transitions.hpp"

namespace mf {
namespace uploader {
namespace detail {
namespace hash_transition {

uint64_t ParseRead(
        const hash_event::HasherStateData & state,
        const uint64_t previous_total_bytes_read,
        const uint64_t bytes_read,
        const char * const buffer
    )
{
    if (bytes_read == 0)
        return previous_total_bytes_read;

    auto & hasher = state->primary_hasher;
    auto & chunk_hasher = state->chunk_hasher;
    auto & chunk_hashes = state->chunk_hashes;

    const auto & chunk_ranges = state->chunk_ranges;
    const auto current_chunk_num = chunk_hashes.size();
    const auto read_start = previous_total_bytes_read;
    const auto read_end = read_start + bytes_read;
    const auto chunk_range = chunk_ranges.at(current_chunk_num);

    hasher.Update(bytes_read, buffer);

    // New range must occur when read_end == chunk range end, not on start.
    assert (read_start < chunk_range.second);

    if (read_end < chunk_range.second)
    {
        // Current read is completely in current range
        chunk_hasher.Update(bytes_read, buffer);

        return previous_total_bytes_read + bytes_read;
    }
    else
    {
        // If read :  2 3 | 4 5 6 7
        // Chunk span first: 2,4
        // Chunk span second: 4,8
        // Bytes remaining in current chunk: 2 ( 4 - 2 )
        // Bytes remaining in next chunk: 4 ( 6 - current chunk bytes )
        const auto bytes_to_first_chunk =
            chunk_range.second - read_start;
        const auto unparsed_bytes =
            bytes_read - bytes_to_first_chunk;

        // Complete this chunk
        chunk_hasher.Update(bytes_to_first_chunk, buffer);
        chunk_hashes.push_back( chunk_hasher.Digest() );
        chunk_hasher = ::mf::utils::Sha256Hasher();

        // Bytes are now processed, increment read position
        const auto new_total_bytes_read =
            previous_total_bytes_read + bytes_to_first_chunk;

        // Process remaining read data recursively if any left
        if (unparsed_bytes > 0)
        {
            return ParseRead(
                state,
                new_total_bytes_read,
                unparsed_bytes,
                buffer + bytes_to_first_chunk);
        }
        else
        {
            return new_total_bytes_read;
        }
    }
}

}  // namespace hash_transition
}  // namespace detail
}  // namespace uploader
}  // namespace mf
