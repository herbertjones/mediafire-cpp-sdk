/**
 * @file api/actions/detail/clone_cloud_tree_impl.hpp
 * @author Herbert Jones
 * @brief Implemetation for CloneCloudTree
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include "../clone_cloud_tree.hpp"

namespace mf
{
namespace api
{

template <typename RequestType>
CloneCloudTreeVersioned<RequestType>::CloneCloudTreeVersioned(
        mf::api::SessionMaintainer * stm,
        typename Base::Callback callback,
        std::string folderkey)
        : Base(stm, callback, ConcurrentRequests)
{
    // Add root folderkey.
    folders_to_scan_.push_back(folderkey);
}

// This adds coroutine keywords.  Use unyield to remove them in header files.
#include "boost/asio/yield.hpp"

template <typename RequestType>
void CloneCloudTreeVersioned<RequestType>::operator()()
{
    reenter(this)
    {
        for (;;)
        {
            // Stop when all folders have been scanned.
            if (folders_to_scan_.empty())
                return Base::ReturnSuccess();

            // Remove actions for next run
            folder_actions_.clear();
            file_actions_.clear();

            for (const auto & next_folderkey : folders_to_scan_)
            {
                // Get folders first
                this->EnqueueAction(folder_actions_, next_folderkey,
                                    GetFolderContentType::Folders);

                // Then get files
                this->EnqueueAction(file_actions_, next_folderkey,
                                    GetFolderContentType::Files);
            }

            // Don't rescan these.
            folders_to_scan_.clear();

            yield this->WaitForEnqueued();

            // Handle result after yield
            for (auto & folder_action : folder_actions_)
            {
                if (folder_action->GetActionResult() != ActionResult::Success)
                    return Base::ReturnFailure(folder_action);

                for (const auto & folder : folder_action->folders)
                {
                    folders.push_back(
                            std::make_pair(folder_action->Folderkey(), folder));
                    folders_to_scan_.push_back(folder.folderkey);
                }
            }

            for (auto & file_action : file_actions_)
            {
                if (file_action->GetActionResult() != ActionResult::Success)
                    return Base::ReturnFailure(file_action);

                for (const auto & file : file_action->files)
                {
                    files.push_back(
                            std::make_pair(file_action->Folderkey(), file));
                }
            }
        }
    }
}

// This removes coroutine keywords.
#include "boost/asio/unyield.hpp"

}  // namespace api
}  // namespace mf
