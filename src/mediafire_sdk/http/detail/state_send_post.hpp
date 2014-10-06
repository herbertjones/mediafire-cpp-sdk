/**
 * @file state_send_post.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <chrono>
#include <memory>
#include <sstream>

#include "boost/asio.hpp"
#include "boost/msm/front/state_machine_def.hpp"

#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/detail/race_preventer.hpp"
#include "mediafire_sdk/http/detail/types.hpp"
#include "mediafire_sdk/http/error.hpp"

namespace mf {
namespace http {
namespace detail {

class SendPostData
{
public:
    SendPostData(
            const uint64_t post_interface_size
        ) :
        cancelled(false),
        post_interface_size(post_interface_size),
        bytes_read_from_interface(0)
    {}

    bool cancelled;

    const uint64_t post_interface_size;
    uint64_t bytes_read_from_interface;
};
using SendPostDataPointer = std::shared_ptr<SendPostData>;

template <typename FSM>
void PostViaInterfaceDelayCallback(
        FSM & fsm,
        SendPostDataPointer state_data,
        mf::http::SharedBuffer::Pointer post_data
    )
{
    // Stop processing if actions cancelled.
    if (state_data->cancelled == true)
        return;

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
            [fsmp, race_preventer, start_time, state_data, post_data](
                   const boost::system::error_code& ec,
                   std::size_t bytes_transferred
                )
            {
                // post_data passed in to prevent it from being freed
                HandlePostWrite(*fsmp, state_data, race_preventer, start_time,
                    bytes_transferred, ec);
            });
    }
}

template <typename FSM>
void PostViaInterface(
        FSM & fsm,
        SendPostDataPointer state_data,
        TimePoint last_write_start,
        TimePoint last_write_end
    )
{
    using mf::http::http_error;

    // Stop processing if actions cancelled.
    if (state_data->cancelled == true)
        return;

    mf::http::SharedBuffer::Pointer post_data;

    auto post_interface_ = fsm.get_post_interface();
    assert(post_interface_);

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
                    http_error::PostInterfaceReadFailure ),
                ss.str()
            });
    }

    if ( ! post_data || post_data->Size() == 0 )
    {
        // Allow interface to free.
        post_interface_.reset();

        if ( state_data->bytes_read_from_interface !=
            state_data->post_interface_size )
        {
            std::stringstream ss;
            ss << "POST data unexpected size.";
            ss << " Expected: " << state_data->post_interface_size;
            ss << " Received: " << state_data->bytes_read_from_interface;
            fsm.ProcessEvent(ErrorEvent{
                    make_error_code(
                        http_error::PostInterfaceReadFailure ),
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
        state_data->bytes_read_from_interface += post_data->Size();

        auto fsmp = fsm.AsFrontShared();

        // Delay behavior:
        fsm.SetTransactionDelayTimer( last_write_start, last_write_end );
        std::function<void()> strand_wrapped(
            [fsmp, state_data, post_data]()
            {
                PostViaInterfaceDelayCallback(*fsmp, state_data, post_data);
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
        SendPostDataPointer state_data,
        RacePreventer race_preventer,
        const TimePoint start_time,
        const std::size_t bytes_transferred,
        const boost::system::error_code& err
    )
{
    using mf::http::http_error;

    // Stop processing if actions cancelled.
    if (state_data->cancelled == true)
        return;

    // Skip if cancelled due to timeout.
    if ( ! race_preventer.IsFirst() ) return;

    fsm.ClearAsyncTimeout();  // Must stop timeout timer.

    if (auto bwa = fsm.get_bw_analyser())
    {
        bwa->RecordOutgoingBytes(bytes_transferred, start_time, sclock::now());
    }

    if (!err)
    {
        if (fsm.get_post_interface())
        {
            PostViaInterface( fsm, state_data, start_time, sclock::now());
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
    }
};

class SendPost : public boost::msm::front::state<>
{
public:
    template <typename Event, typename FSM>
    void on_entry(Event const &, FSM & fsm)
    {
        auto post_data = fsm.get_post_data();
        if ( post_data )
        {
            // Must prime timeout for async actions.
            auto race_preventer = fsm.SetAsyncTimeout("write request post",
                fsm.get_timeout_seconds());
            auto fsmp = fsm.AsFrontShared();
            auto start_time = sclock::now();
            auto state_data = std::make_shared<SendPostData>(0);
            state_data_ = state_data;

            boost::asio::async_write(
                *fsm.get_socket_wrapper(),
                boost::asio::buffer(post_data->Data(), post_data->Size()),
                [fsmp, state_data, race_preventer, start_time](
                       const boost::system::error_code& ec,
                       std::size_t bytes_transferred
                    )
                {
                    // post_data passed in to prevent it from being freed
                    HandlePostWrite(*fsmp, state_data, race_preventer,
                        start_time, bytes_transferred, ec);
                });
        }
        else
        {
            auto post_interface_ = fsm.get_post_interface();
            auto state_data = std::make_shared<SendPostData>(
                post_interface_->PostDataSize());
            state_data_ = state_data;
            const auto now = std::chrono::steady_clock::now();
            PostViaInterface( fsm, state_data, now, now );
        }
    }

    template <typename Event, typename FSM>
    void on_exit(Event const&, FSM &)
    {
        assert(state_data_);
        state_data_->cancelled = true;
        state_data_.reset();
    }

private:
    SendPostDataPointer state_data_;
};



}  // namespace detail
}  // namespace http
}  // namespace mf
