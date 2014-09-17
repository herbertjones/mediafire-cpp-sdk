/**
 * @file transition_poll_upload.hpp
 * @author Herbert Jones
 * @brief Transition for poll upload state
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <cassert>
#include <string>
#include <system_error>

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/upload/poll_upload.hpp"

#include "mediafire_sdk/uploader/error.hpp"
#include "mediafire_sdk/uploader/detail/upload_events.hpp"

namespace {
const int poll_upload_retry_timeout_seconds = 1;
}  // namespace

namespace mf {
namespace uploader {
namespace detail {
namespace upload_transition {

// Forward declarations
template <typename FSM>
void StartPoll(
        const std::string & upload_key,
        FSM & fsm
    );


template <typename FSM>
void HandlePollCompleteResponse(
        FSM & fsm,
        const mf::api::upload::poll_upload::Response & response
    )
{
    if (response.quickkey)
    {
        fsm.ProcessEvent(event::PollComplete{
            *response.quickkey
            });
    }
    else
    {
        fsm.ProcessEvent(event::Error{
            make_error_code(api::errc::ContentInvalidData),
            "Successful response missing quickkey"
            });
    }
}

template <typename FSM>
void RetryPoll(
        const std::string & upload_key,
        std::shared_ptr<FSM> fsmp,
        const boost::system::error_code &err
    )
{
    if (!err)
    {
        StartPoll(upload_key, *fsmp);
    }
}

template <typename FSM>
void HandlePollResponse(
        const std::string & upload_key,
        FSM & fsm,
        const mf::api::upload::poll_upload::Response & response
    )
{
    // if result is negative, it indicates a failure
    if ( response.result < 0 )
    {
        fsm.ProcessEvent(event::Error{
            std::error_code(response.result, poll_result_category()),
            "Filsize unavailable."
            });
    }
    else if( response.fileerror != 0 )
    {
        fsm.ProcessEvent(event::Error{
            std::error_code(response.fileerror, poll_result_category()),
            "Filsize unavailable."
            });
    }
    else if( response.status == 99 )
    {
        HandlePollCompleteResponse(fsm, response);
    }
    else
    {
        auto timer = fsm.Timer();

        timer->expires_from_now( std::chrono::seconds(
                poll_upload_retry_timeout_seconds) );

        timer->async_wait( boost::bind( &RetryPoll<FSM>, upload_key,
                fsm.AsFrontShared(), boost::asio::placeholders::error));
    }
}

template <typename FSM>
void StartPoll(
        const std::string & upload_key,
        FSM & fsm
    )
{
    if ( upload_key.empty() )
    {
        assert(!"Reached poll upload without upload key");
        fsm.ProcessEvent(event::Error{
            make_error_code(uploader::errc::LogicError),
            "Filsize unavailable."
            });
        return;
    }

    auto fsmp = fsm.AsFrontShared();

    fsm.GetSessionMaintainer()->Call(
        mf::api::upload::poll_upload::Request(upload_key),
        [fsmp, upload_key](const mf::api::upload::poll_upload::Response & response)
        {
            HandlePollResponse(upload_key, *fsmp, response);
        });
}

struct PollUpload
{
    template <typename Event, typename FSM, typename SourceState, typename TargetState>
    void operator()(
            Event const & evt,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        StartPoll(evt.upload_key, fsm);
    }
};

}  // namespace upload_transition
}  // namespace detail
}  // namespace uploader
}  // namespace mf
