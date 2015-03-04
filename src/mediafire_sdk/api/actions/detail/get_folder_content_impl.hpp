/**
 * @file api/actions/detail/get_folder_content_impl.hpp
 * @author Herbert Jones
 * @brief Implementation details for GetFolderContent
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include "../get_folder_content.hpp"

namespace mf
{
namespace api
{

template <typename RequestType>
GetFolderContentVersioned<RequestType>::GetFolderContentVersioned(
        mf::api::SessionMaintainer * stm,
        typename Base::Callback callback,
        std::string folderkey,
        FilesFoldersOrBoth type)
        : Base(stm,
               callback,
               2 /* there can never be more than 2 concurrent requests */),
          folderkey_(folderkey),
          type_(type),
          chunk_number_(1),  // Chunks start at 1.
          file_request_remaining_(true),
          folder_request_remaining_(true)
{
}

template <typename RequestType>
bool GetFolderContentVersioned<RequestType>::EnqueueFileRequest()
{
    if (file_request_remaining_ && type_ != FilesFoldersOrBoth::Folders)
    {
        current_file_request_.reset(new Request{
                folderkey_,                  // Folderkey
                chunk_number_,               // Chunk count
                Request::ContentType::Files  // file or folder
        });

        // Make response and yield
        this->EnqueueApi(*current_file_request_, file_response_);

        return true;
    }
    else
    {
        return false;
    }
}

template <typename RequestType>
bool GetFolderContentVersioned<RequestType>::EnqueueFolderRequest()
{
    if (folder_request_remaining_ && type_ != FilesFoldersOrBoth::Files)
    {
        current_folder_request_.reset(new Request{
                folderkey_,                    // Folderkey
                chunk_number_,                 // Chunk count
                Request::ContentType::Folders  // file or folder
        });

        // Make response and yield
        this->EnqueueApi(*current_folder_request_, folder_response_);

        return true;
    }
    else
    {
        return false;
    }
}

template <typename RequestType>
typename GetFolderContentVersioned<RequestType>::HandledFailure
GetFolderContentVersioned<RequestType>::HandleFileResponse()
{
    if (file_request_remaining_ && type_ != FilesFoldersOrBoth::Folders)
    {
        // Check response on return.
        if (file_response_.error_code)
        {
            this->ReturnFailure(file_response_.error_code,
                                file_response_.error_string);
            return HandledFailure::Yes;
        }

        // Extract files on success.
        for (const auto & file : file_response_.files)
        {
            files.push_back(file);
        }

        if (file_response_.chunks_remaining
            == Request::ChunksRemaining::LastChunk)
        {
            file_request_remaining_ = false;
        }
    }
    return HandledFailure::No;
}

template <typename RequestType>
typename GetFolderContentVersioned<RequestType>::HandledFailure
GetFolderContentVersioned<RequestType>::HandleFolderResponse()
{
    if (folder_request_remaining_ && type_ != FilesFoldersOrBoth::Files)
    {
        // Check response on return.
        if (folder_response_.error_code)
        {
            this->ReturnFailure(folder_response_.error_code,
                                folder_response_.error_string);
            return HandledFailure::Yes;
        }

        // Extract folders on success.
        for (const auto & folder : folder_response_.folders)
        {
            folders.push_back(folder);
        }

        if (folder_response_.chunks_remaining
            == Request::ChunksRemaining::LastChunk)
        {
            folder_request_remaining_ = false;
        }
    }
    return HandledFailure::No;
}

// This adds coroutine keywords.  Use unyield to remove them in header files.
#include "boost/asio/yield.hpp"

template <typename RequestType>
void GetFolderContentVersioned<RequestType>::operator()()
{
    reenter(this)
    {
        for (;;)
        {
            {
                const bool has_files = EnqueueFileRequest();
                const bool has_folder = EnqueueFolderRequest();

                if (!has_files && !has_folder)
                    return Base::ReturnSuccess();
            }

            yield this->WaitForEnqueued();

            if (HandleFileResponse() == HandledFailure::Yes
                || HandleFolderResponse() == HandledFailure::Yes)
                return;

            ++chunk_number_;  // Increment chunk for next iteration.
        }
    }
}

// This removes coroutine keywords.
#include "boost/asio/unyield.hpp"

}  // namespace api
}  // namespace mf
