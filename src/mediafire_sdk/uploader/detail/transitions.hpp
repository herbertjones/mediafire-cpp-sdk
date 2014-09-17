/**
 * @file transitions.hpp
 * @author Herbert Jones
 * @brief State machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include "mediafire_sdk/uploader/detail/hasher_transitions.hpp"
#include "mediafire_sdk/uploader/detail/transition_check.hpp"
#include "mediafire_sdk/uploader/detail/transition_instant_upload.hpp"
#include "mediafire_sdk/uploader/detail/transition_poll_upload.hpp"
#include "mediafire_sdk/uploader/detail/transition_upload.hpp"

namespace mf {
namespace uploader {
namespace detail {
namespace upload_transition {

struct SaveHash
{
    template <typename FSM, typename SourceState, typename TargetState>
    void operator()(
            hash_event::HashSuccess const & hash_success,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        fsm.SetCompleteHashData(hash_success.sha256_hash, hash_success.state);
    }
};

}  // namespace upload_transition
}  // namespace detail
}  // namespace uploader
}  // namespace mf
