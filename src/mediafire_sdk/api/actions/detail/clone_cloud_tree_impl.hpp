/**
 * @file api/actions/detail/clone_cloud_tree_impl.hpp
 * @author Herbert Jones
 * @brief Implemetation for CloneCloudTree
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include "../clone_cloud_tree.hpp"
#include "mediafire_sdk/utils/container_algorithms.hpp"

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
    folders_to_scan_.push_back(
            std::make_pair(folderkey, FilesFoldersOrBoth::Both));
}

template <typename RequestType>
CloneCloudTreeVersioned<RequestType>::CloneCloudTreeVersioned(
        mf::api::SessionMaintainer * stm,
        typename Base::Callback callback,
        std::vector<std::string> folderkeys)
        : Base(stm, callback, ConcurrentRequests)
{
    for (auto & folderkey : folderkeys)
    {
        folders_to_scan_.push_back(
                std::make_pair(folderkey, FilesFoldersOrBoth::Both));
    }
}

template <typename RequestType>
CloneCloudTreeVersioned<RequestType>::CloneCloudTreeVersioned(
        mf::api::SessionMaintainer * stm,
        typename Base::Callback callback,
        FolderWorkList work_list)
        : Base(stm, callback, ConcurrentRequests)
{
    folders_to_scan_.swap(work_list);
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

            EnqueueWork();

            // Don't rescan these.
            folders_to_scan_.clear();

            yield this->WaitForEnqueued();

            if (HandleResponse() == ResponseResult::ErrorHandled)
            {
                // Error reported in function.
                return;
            }
        }
    }
}

// This removes coroutine keywords.
#include "boost/asio/unyield.hpp"

template <typename RequestType>
void CloneCloudTreeVersioned<RequestType>::EnqueueWork()
{
    for (const auto & work : folders_to_scan_)
    {
        if (work.second != FilesFoldersOrBoth::Files)
        {
            // Get folders first
            this->EnqueueAction(folder_actions_, work.first,
                                FilesFoldersOrBoth::Folders);
        }

        if (work.second != FilesFoldersOrBoth::Folders)
        {
            // Then get files
            this->EnqueueAction(file_actions_, work.first,
                                FilesFoldersOrBoth::Files);
        }
    }
}

template <typename RequestType>
typename CloneCloudTreeVersioned<RequestType>::ResponseResult
CloneCloudTreeVersioned<RequestType>::HandleResponse()
{
    std::error_code error_code;
    boost::optional<std::string> error_description;

    std::vector<std::string> failed_file_traversal;
    std::vector<std::string> failed_folder_traversal;

    for (auto & file_action : file_actions_)
    {
        if (file_action->GetActionResult() != ActionResult::Success)
        {
            if (!error_code)
            {
                error_code = file_action->GetErrorCode();
                error_description = file_action->GetErrorDescription();
            }

            // Record that folder was unprocessed.
            failed_file_traversal.push_back(file_action->Folderkey());
        }
        else
        {
            for (const auto & file : file_action->files)
                files.push_back(std::make_pair(file_action->Folderkey(), file));
        }
    }

    // Handle result after yield
    for (auto & folder_action : folder_actions_)
    {
        if (folder_action->GetActionResult() != ActionResult::Success)
        {
            if (!error_code)
            {
                error_code = folder_action->GetErrorCode();
                error_description = folder_action->GetErrorDescription();
            }

            // Record that folder was unprocessed.
            failed_folder_traversal.push_back(folder_action->Folderkey());
        }
        else
        {
            for (const auto & folder : folder_action->folders)
            {
                folders.push_back(
                        std::make_pair(folder_action->Folderkey(), folder));
                folders_to_scan_.push_back(std::make_pair(
                        folder.folderkey, FilesFoldersOrBoth::Both));
            }
        }
    }

    if (error_code)
    {
        std::sort(failed_file_traversal.begin(), failed_file_traversal.end());
        std::sort(failed_folder_traversal.begin(),
                  failed_folder_traversal.end());
        std::vector<std::string> failed_both;

        mf::utils::RepartitionIntersection(
                failed_file_traversal, failed_folder_traversal, failed_both);

        for (const auto & folderkey : failed_file_traversal)
            untraversed_folders.push_back(
                    std::make_pair(folderkey, FilesFoldersOrBoth::Files));
        for (const auto & folderkey : failed_folder_traversal)
            untraversed_folders.push_back(
                    std::make_pair(folderkey, FilesFoldersOrBoth::Folders));
        for (const auto & folderkey : failed_both)
            untraversed_folders.push_back(
                    std::make_pair(folderkey, FilesFoldersOrBoth::Both));

        // Report failure
        Base::ReturnFailure(error_code, error_description);

        // Stop processing
        return ResponseResult::ErrorHandled;
    }
    else
    {
        return ResponseResult::Success;
    }
}

}  // namespace api
}  // namespace mf
