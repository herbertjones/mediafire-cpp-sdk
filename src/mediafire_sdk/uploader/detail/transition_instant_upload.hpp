/**
 * @file transition_instant_upload.hpp
 * @author Herbert Jones
 * @brief State machine instant upload transition
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <iostream>

#include "boost/variant/apply_visitor.hpp"

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/upload/instant.hpp"

#include "mediafire_sdk/uploader/upload_request.hpp"
#include "mediafire_sdk/uploader/detail/upload_events.hpp"

namespace mf {
namespace uploader {
namespace detail {
namespace upload_transition {

struct DoInstantUpload
{
    template <typename Event, typename FSM, typename SourceState, typename TargetState>
    void operator()(
            Event const &,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        namespace instant = mf::api::upload::instant;

        auto request = instant::Request( fsm.Filename(), fsm.Hash(),
            fsm.Filesize());

        if (fsm.OnDuplicateAction() == OnDuplicateAction::Replace)
            request.SetActionOnDuplicate(instant::ActionOnDuplicate::Replace);


        class Visitor : public boost::static_visitor<>
        {
        public:
            Visitor( instant::Request * request) : request_(request) {}

            void operator()(mf::uploader::detail::ParentFolderKey & variant) const
            {
                request_->SetTargetParentFolderkey(variant.key);
            }

            void operator()(mf::uploader::detail::CloudPath & variant) const
            {
                request_->SetTargetPath(variant.path);
            }

        private:
            instant::Request * request_;
        };
        UploadTarget target_folder = fsm.TargetFolder();
        boost::apply_visitor(Visitor(&request), target_folder);

        auto fsmp = fsm.AsFrontShared();

        fsm.GetSessionMaintainer()->Call(
            request,
            [fsmp](const instant::Response & response)
            {
                if (response.error_code)
                {
                    fsmp->ProcessEvent(event::Error{response.error_code,
                        "Failed to instant upload file."});
                }
                else
                {
                    fsmp->ProcessEvent(event::InstantSuccess{
                        response.quickkey });
                }
            });
    }
};

}  // namespace upload_transition
}  // namespace detail
}  // namespace uploader
}  // namespace mf
