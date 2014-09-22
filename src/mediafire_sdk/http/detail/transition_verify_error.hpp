/**
 * @file transition_config.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 *
 * Detailed message...
 */
#pragma once

#include <chrono>

#include "mediafire_sdk/http/error.hpp"
#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/detail/types.hpp"

namespace mf {
namespace http {
namespace detail {

struct VerifyErrorAction
{
    template <typename Event, typename FSM,typename SourceState,typename TargetState>
    void operator()(
            Event const & evt,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        assert( fsm.get_event_strand()->running_in_this_thread() );

        // Close connection
        fsm.Disconnect();

        auto timeout = fsm.get_request_creation_time() +
            std::chrono::seconds(fsm.get_timeout_seconds());

        if (evt.code == mf::http::http_error::IoTimeout && sclock::now() < timeout)
        {
            // Restart everything
            fsm.ProcessEvent(RestartEvent{});
        }
        else
        {
            // Pass error on
            fsm.ProcessEvent(evt);
        }
    }
};

}  // namespace detail
}  // namespace http
}  // namespace mf
