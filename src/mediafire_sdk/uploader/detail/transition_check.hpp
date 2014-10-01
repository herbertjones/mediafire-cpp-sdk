/**
 * @file transition_check.hpp
 * @author Herbert Jones
 * @brief State machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <iostream>

#include "boost/variant/apply_visitor.hpp"

#include "mediafire_sdk/api/upload/check.hpp"
#include "mediafire_sdk/uploader/error.hpp"
#include "mediafire_sdk/uploader/upload_request.hpp"
#include "mediafire_sdk/uploader/detail/types.hpp"
#include "mediafire_sdk/uploader/detail/upload_events.hpp"
#include "mediafire_sdk/uploader/detail/upload_target.hpp"
#include "mediafire_sdk/utils/string.hpp"

namespace mf {
namespace uploader {
namespace detail {
namespace upload_transition {

template<typename FSM>
void HandleCheck(
        FSM & fsm,
        const mf::api::upload::check::Response & response
    )
{
    assert( ! fsm.Filename().empty());

    namespace chk = mf::api::upload::check;

    if (response.file_exists == chk::FilenameInFolder::Yes
        && response.hash_different == chk::FileExistsWithDifferentHash::No
        && response.duplicate_quickkey
        )
    {
        // Filename same and hash same.  We are already done here.
        fsm.ProcessEvent( event::AlreadyUploaded{ *response.duplicate_quickkey,
            fsm.Filename() });
    }
    else if (response.storage_limit_exceeded == chk::StorageLimitExceeded::Yes)
    {
        // Can't upload, insufficient space.

        fsm.ProcessEvent( event::Error{
                make_error_code(mf::uploader::errc::InsufficientCloudStorage),
                "Account lacks sufficient storage for upload to cloud"
            } );
    }
    else if (response.file_exists == chk::FilenameInFolder::Yes
        &&  fsm.OnDuplicateAction() != OnDuplicateAction::Replace
        && fsm.OnDuplicateAction() != OnDuplicateAction::AutoRename )
    {
        switch (fsm.OnDuplicateAction())
        {
            case OnDuplicateAction::Fail:
                fsm.ProcessEvent( event::Error{
                    make_error_code(mf::uploader::errc::FileExistInFolder),
                    "File already exists in folder."
                    } );
                break;
            default:
                assert(!"Unhandled duplicate action");
        }
    }
    else if (response.hash_exists == chk::HashAlreadyInSystem::Yes)
    {
        fsm.ProcessEvent( event::InstantUpload{} );
    }
    else if (fsm.ChunkRanges().size() > 1)
    {
        if ( ! response.resumable )
        {
            assert(!"bitmap missing when it should exist");
            fsm.ProcessEvent( event::NeedsSingleUpload{} );
        }
        else
        {
            const auto & resumable = *response.resumable;
            if (resumable.number_of_units != fsm.ChunkRanges().size())
            {
                assert(!"unit count does not match chunk count");
                fsm.ProcessEvent( event::NeedsSingleUpload{} );
            }
            else
            {
                fsm.SetBitmap(resumable.words);

                // Do upload
                fsm.ProcessEvent( event::NeedsChunkUpload{} );
            }
        }
    }
    else
    {
        fsm.ProcessEvent( event::NeedsSingleUpload{} );
    }
}

struct Check
{
    template <typename Event, typename FSM, typename SourceState, typename TargetState>
    void operator()(
            Event const &,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        auto request = ::mf::api::upload::check::Request(fsm.Filename());
        request.SetHash(fsm.Hash());
        request.SetFilesize(fsm.Filesize());

        // Set target folderpath
        class Visitor : public boost::static_visitor<>
        {
        public:
            Visitor(
                    ::mf::api::upload::check::Request * request
                ) :
                request_(request) {}

            void operator()(ParentFolderKey variant) const
            {
                request_->SetTargetParentFolderkey(variant.key);
            }

            void operator()(CloudPath variant) const
            {
                request_->SetPath(mf::utils::path_to_utf8(variant.path));
            }

        private:
            ::mf::api::upload::check::Request * request_;
        };

        auto target = fsm.TargetFolder();
        boost::apply_visitor(Visitor(&request), target);

        if (fsm.ChunkRanges().size() > 1)
        {
            // If multiple chunks, we should set resumable
            request.SetResumable(::mf::api::upload::check::Resumable::Resumable);
        }

        auto fsmp = fsm.AsFrontShared();

        fsm.GetSessionMaintainer()->Call(
            request,
            [fsmp](const mf::api::upload::check::Response & response)
            {
                if (response.error_code)
                {
                    std::string error_str = "unknown error";
                    if (response.error_string)
                        error_str = *response.error_string;

                    fsmp->ProcessEvent(
                        event::Error{
                            response.error_code,
                            error_str
                        });
                }
                else
                {
                    HandleCheck(*fsmp, response);
                }
            }
        );
    }
};


}  // namespace upload_transition
}  // namespace detail
}  // namespace uploader
}  // namespace mf
