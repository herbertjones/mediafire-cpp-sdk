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
#include <sstream>

#include "boost/asio.hpp"

#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/detail/race_preventer.hpp"
#include "mediafire_sdk/http/detail/types.hpp"
#include "mediafire_sdk/http/error.hpp"

namespace mf {
namespace http {
namespace detail {

using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

template <typename FSM>
void PostViaInterfaceDelayCallback(
        FSM & fsm,
        mf::http::SharedBuffer::Pointer post_data
    )
{
    // Ensure a cancellation doesn't mess up the state due to async
    // timer.
    if ( fsm.get_transmission_delay_timer_enabled() )
    {
        fsm.set_transmission_delay_timer_enabled(false);

        // Must prime timeout for async actions.
        auto race_preventer = fsm.SetAsyncTimeout("write request post",
            fsm.get_timeout_seconds());
        auto fsmp = fsm.AsFrontShared();
        auto start_time = sclock::now();

        asio::async_write(
            *fsm.get_socket_wrapper(),
            asio::buffer(post_data->Data(), post_data->Size()),
            [fsmp, race_preventer, start_time](
                   const boost::system::error_code& ec,
                   std::size_t bytes_transferred
                )
            {
                // post_data passed in to prevent it from being freed
                HandlePostWrite(*fsmp, race_preventer, start_time,
                    bytes_transferred, ec);
            });
    }
}

template <typename FSM>
void PostViaInterface(
        FSM & fsm,
        TimePoint last_write_start,
        TimePoint last_write_end
    )
{
    using mf::http::http_error;

    mf::http::SharedBuffer::Pointer post_data;

    auto post_interface_ = fsm.get_post_interface();

    try {
        post_data = post_interface_->RetreivePostDataChunk();
    }
    catch( const std::exception & err )
    {
        std::stringstream ss;
        ss << "Failure to retrieve POST data from interface.";
        ss << " Error: " << err.what();
        fsm.ProcessEvent(ErrorEvent{
                make_error_code(
                    http_error::VariablePostInterfaceFailure ),
                ss.str()
            });
    }

    if ( ! post_data || post_data->Size() == 0 )
    {
        // Allow interface to free.
        post_interface_.reset();

        if ( fsm.get_post_interface_read_bytes() != fsm.get_post_interface_size() )
        {
            std::stringstream ss;
            ss << "POST data unexpected size.";
            ss << " Expected: " << fsm.get_post_interface_size();
            ss << " Received: " << fsm.get_post_interface_read_bytes();
            fsm.ProcessEvent(ErrorEvent{
                    make_error_code(
                        http_error::VariablePostInterfaceFailure ),
                    ss.str()
                });
        }
        else
        {
            fsm.ProcessEvent(PostSent{});
        }
    }
    else
    {
        fsm.increment_post_interface_read_bytes(post_data->Size());

        auto fsmp = fsm.AsFrontShared();

        // Delay behavior:
        fsm.SetTransactionDelayTimer( last_write_start, last_write_end );
        std::function<void()> strand_wrapped(
            [fsmp, post_data]()
            {
                PostViaInterfaceDelayCallback(*fsmp, post_data);
            });

        fsm.get_transmission_delay_timer()->async_wait(
            fsm.get_event_strand()->wrap(
                [fsmp, strand_wrapped](
                       const boost::system::error_code& ec
                    )
                {
                    fsmp->SetTransactionDelayTimerWrapper(strand_wrapped, ec);
                }));
    }
}

template <typename FSM>
void HandlePostWrite(
        FSM & fsm,
        RacePreventer race_preventer,
        const TimePoint start_time,
        const std::size_t bytes_transferred,
        const boost::system::error_code& err
    )
{
    using mf::http::http_error;

    // Skip if cancelled due to timeout.
    if ( ! race_preventer.IsFirst() ) return;

    fsm.ClearAsyncTimeout();  // Must stop timeout timer.

    if (fsm.get_bw_analyser())
    {
        fsm.get_bw_analyser()->RecordOutgoingBytes( bytes_transferred, start_time,
            sclock::now() );
    }

    if (!err)
    {
        if (fsm.get_post_interface())
        {
            PostViaInterface( fsm, start_time, sclock::now() );
        }
        else
        {
            fsm.ProcessEvent(PostSent{});
        }
    }
    else
    {
        std::stringstream ss;
        ss << "Failure writing POST. URL: " << fsm.get_url();
        ss << " Error: " << err.message();
        fsm.ProcessEvent(
            ErrorEvent{
                make_error_code(
                    http_error::WriteFailure ),
                ss.str()
            });
    }
}

struct SendPostAction
{
    template <typename Event, typename FSM,typename SourceState,typename TargetState>
    void operator()(
            Event const &,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        auto post_data = fsm.get_post_data();
        if ( post_data )
        {
            // Must prime timeout for async actions.
            auto race_preventer = fsm.SetAsyncTimeout("write request post",
                fsm.get_timeout_seconds());
            auto fsmp = fsm.AsFrontShared();
            auto start_time = sclock::now();

            boost::asio::async_write(
                *fsm.get_socket_wrapper(),
                boost::asio::buffer(post_data->Data(), post_data->Size()),
                [fsmp, race_preventer, start_time](
                       const boost::system::error_code& ec,
                       std::size_t bytes_transferred
                    )
                {
                    // post_data passed in to prevent it from being freed
                    HandlePostWrite(*fsmp, race_preventer, start_time,
                        bytes_transferred, ec);
                });
        }
        else
        {
            auto post_interface_ = fsm.get_post_interface();
            fsm.set_post_interface_size(post_interface_->PostDataSize());

            const auto now = std::chrono::steady_clock::now();
            PostViaInterface( fsm, now, now );
        }
    }
};

}  // namespace detail
}  // namespace http
}  // namespace mf
