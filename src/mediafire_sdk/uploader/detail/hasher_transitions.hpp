/**
 * @file hasher_transitions.hpp
 * @author Herbert Jones
 * @brief Transitions for the hasher
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include "boost/filesystem.hpp"

#include "mediafire_sdk/utils/error.hpp"
#include "mediafire_sdk/uploader/detail/hasher_events.hpp"

namespace mf {
namespace uploader {
namespace detail {
namespace hash_transition {

uint64_t ParseRead(
        const hash_event::HasherStateData & state,
        const uint64_t previous_total_bytes_read,
        const uint64_t bytes_read,
        const char * const buffer
    );

template <typename FSM>
bool VerifyFileUnchanged(
        const hash_event::HasherStateData & state,
        FSM & fsm
    )
{
    using mf::utils::file_io_error;

    const auto filepath = state->filepath;
    const auto original_filesize = state->filesize;
    const auto original_mtime = state->mtime;

    boost::system::error_code bec;

    // Check mtime
    const std::time_t mtime = boost::filesystem::last_write_time(
        state->filepath, bec);

    if (bec)
    {
        std::error_code ec(bec.value(), std::system_category());
        fsm.process_event( hash_event::Error{
            state,
            ec,
            "Unable to get file mtime."
            } );
        return false;
    }

    if (mtime != original_mtime)
    {
        fsm.process_event( hash_event::Error{
            state,
            make_error_code(file_io_error::FileModified),
            "Mtime changed from expected value."
            } );
        return false;
    }

    const uint64_t filesize = boost::filesystem::file_size(filepath, bec);

    if (bec)
    {
        std::error_code ec(bec.value(), std::system_category());
        fsm.process_event( hash_event::Error{
            state,
            ec,
            "Unable to get filesize."
            } );
        return false;
    }

    if (filesize != original_filesize)
    {
        fsm.process_event( hash_event::Error{
            state,
            make_error_code(file_io_error::FileModified),
            "Filesize changed from expected value."
            } );
        return false;
    }

    return true;
}

struct OpenFile
{
    template <typename FSM,typename SourceState,typename TargetState>
    void operator()(
            hash_event::StartHash const & src_evt,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        auto & state = src_evt.state;
        auto ec = std::error_code();
        auto file_io = mf::utils::FileIO::Open(state->filepath, "r", &ec);

        if (ec)
        {
            fsm.process_event( hash_event::Error{
                state,
                ec,
                "Unable to open file."
                });
        }
        else
        {
            fsm.process_event( hash_event::ReadNext{
                state,
                file_io,
                0
                });
        }
    }
};

struct ReadFile
{
    template <typename FSM,typename SourceState,typename TargetState>
    void operator()(
            hash_event::ReadNext const & src_evt,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        using mf::utils::file_io_error;

        auto & state = src_evt.state;
        auto & hasher = state->primary_hasher;

        const auto & file_io = src_evt.file_io;
        const auto & previous_total_bytes_read = src_evt.read_byte_pos;

        if ( VerifyFileUnchanged(state, fsm) )
        {
            const int buf_size = 1024 * 8;
            char buffer[buf_size];
            std::error_code ec;

            const auto bytes_read = file_io->Read(buffer, buf_size, &ec);

            if (ec == file_io_error::EndOfFile)
            {
                ParseRead(
                    state,
                    previous_total_bytes_read,
                    bytes_read,
                    buffer );
                assert( state->chunk_ranges.size() ==
                    state->chunk_hashes.size() );
                hash_event::HashSuccess evt = {
                    state,
                    hasher.Digest()
                };

                if ( VerifyFileUnchanged(state, fsm) )
                {
                    fsm.process_event( evt );
                }
            }
            else if (ec)
            {
                fsm.process_event( hash_event::Error{
                    state,
                    ec,
                    "Error occured during file read."
                    } );
            }
            else
            {
                auto total_bytes_read = ParseRead(
                    state,
                    previous_total_bytes_read,
                    bytes_read,
                    buffer );

                auto shared_fsm = fsm.shared_from_this();
                FSM * fsm_ptr = &fsm;

                hash_event::ReadNext evt ={
                    state,
                    src_evt.file_io,
                    total_bytes_read
                };

                state->io_service->post(
                    [shared_fsm, fsm_ptr, evt]()
                    {
                        fsm_ptr->process_event(evt);
                    });
            }
        }
    }
};

}  // namespace hash_transition
}  // namespace detail
}  // namespace uploader
}  // namespace mf
