#pragma once

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/folder/get_content.hpp"

#include "coroutine.hpp"

#include "boost/coroutine/all.hpp"

namespace mf
{
namespace api
{

template <typename TRequest>
class GetFolderContents : public Coroutine
{
public:
    // enum class to determine if you want to get the files and/or folders of
    // the folder.
    enum class FilesOrFoldersOrBoth
    {
        Files,
        Folders,
        Both
    };

public:
    // Some convenience typedefs
    using RequestType = TRequest;
    using ResponseType = typename RequestType::ResponseType;

    using ContentType = typename RequestType::ContentType;

    using File = typename ResponseType::File;
    using Folder = typename ResponseType::Folder;

    // The struct for the errors we might return
    struct ErrorType
    {
        ErrorType(const std::string & folder_key,
                  FilesOrFoldersOrBoth files_or_folders_or_both,
                  uint32_t chunk_num,
                  const std::error_code & error_code,
                  const std::string & error_string);

        std::string folder_key;
        FilesOrFoldersOrBoth files_or_folders_or_both;
        uint32_t chunk_num;
        std::error_code error_code;
        std::string error_string;
    };

    using CallbackType = std::function<void(
            const std::vector<typename ResponseType::File> & files,
            const std::vector<typename ResponseType::Folder> & folders,
            const std::vector<ErrorType> & errors)>;

public:
    /**
     *  @brief Create an instance and get us the shared pointer to the created
     *instance.
     *
     *  @return std::shared_ptr Shared pointer to the created instance.
     **/
    static std::shared_ptr<GetFolderContents> Create(
            SessionMaintainer * stm,
            const std::string & folder_key,
            const FilesOrFoldersOrBoth & files_or_folders_or_both,
            CallbackType && callback);

    /**
     *  @brief Starts/resumes the coroutine.
     */
    void operator()() override;

private:
    /**
     *  @brief  Private constructor.
     **/
    GetFolderContents(SessionMaintainer * stm,
                      const std::string & folder_key,
                      const FilesOrFoldersOrBoth & files_or_folders_or_both,
                      CallbackType && callback);

private:
    SessionMaintainer * stm_;

    std::string folder_key_;

    std::vector<File> files_;
    std::vector<Folder> folders_;

    std::vector<ErrorType> errors_;

    FilesOrFoldersOrBoth files_or_folders_or_both_;

    CallbackType callback_;
    bool file_chunks_remaining_ = false;
    bool folder_chunks_remaining_ = false;

    push_type coro_{
            [this](pull_type & yield)
            {
                auto self = shared_from_this();  // Hold a reference to our
                                                 // object until the coroutine
                                                 // is complete, otherwise
                                                 // handler will have invalid
                                                 // reference to this because
                                                 // the base object has
                                                 // disappeared from scope

                // The chunk number for each content type of call respectively
                int files_chunk_num = 1;
                int folders_chunk_num = 1;

                // Do we still have work left to queue?
                while (folder_chunks_remaining_ || file_chunks_remaining_)
                {
                    int yield_count = 0;

                    // Files or Both
                    if ((files_or_folders_or_both_
                                 == FilesOrFoldersOrBoth::Files
                         || files_or_folders_or_both_
                                    == FilesOrFoldersOrBoth::Both)
                        && file_chunks_remaining_)
                    {
                        std::function<void(const ResponseType & response)>
                                HandleFolderGetContentsFiles
                                = [this, self](const ResponseType & response)
                        {
                            if (response.error_code)
                            {
                                // If there was an error, insert into vector and
                                // propagate at the callback.
                                std::string error_str
                                        = "No error string provided";
                                if (response.error_string)
                                    error_str = *response.error_string;

                                errors_.push_back(
                                        ErrorType(folder_key_,
                                                  FilesOrFoldersOrBoth::Files,
                                                  response.chunk_number,
                                                  response.error_code,
                                                  error_str));
                            }
                            else
                            {
                                // Insert the list of files from the response
                                // into our own list
                                files_.insert(std::end(files_),
                                              std::begin(response.files),
                                              std::end(response.files));

                                // Set flag if the file chunks are remaining
                                if (response.chunks_remaining
                                    == mf::api::folder::get_content::
                                               ChunksRemaining::LastChunk)
                                    file_chunks_remaining_ = false;
                            }

                            // Resume the coroutine
                            (*this)();
                        };

                        stm_->Call(RequestType(folder_key_,
                                                  files_chunk_num,
                                                  ContentType::Files),
                                   HandleFolderGetContentsFiles);

                        ++yield_count;
                    }

                    // Folders or Both
                    if ((files_or_folders_or_both_
                                 == FilesOrFoldersOrBoth::Folders
                         || files_or_folders_or_both_
                                    == FilesOrFoldersOrBoth::Both)
                        && folder_chunks_remaining_)
                    {
                        std::function<void(const ResponseType & response)>
                                HandleFolderGetContentsFolder
                                = [this, self](const ResponseType & response)
                        {
                            if (response.error_code)
                            {
                                // If there was an error, insert into vector and
                                // propagate at the callback.
                                std::string error_str
                                        = "No error string provided";
                                if (response.error_string)
                                    error_str = *response.error_string;

                                errors_.push_back(
                                        ErrorType(folder_key_,
                                                  FilesOrFoldersOrBoth::Folders,
                                                  response.chunk_number,
                                                  response.error_code,
                                                  error_str));
                            }
                            else
                            {
                                // Insert the list of folders from the response
                                // into our own list
                                folders_.insert(std::end(folders_),
                                                std::begin(response.folders),
                                                std::end(response.folders));

                                // Set flag if folder chunks are remaining
                                if (response.chunks_remaining
                                    == mf::api::folder::get_content::
                                               ChunksRemaining::LastChunk)
                                    folder_chunks_remaining_ = false;
                            }

                            // Resume the coroutine
                            (*this)();
                        };

                        stm_->Call(RequestType(folder_key_,
                                                  folders_chunk_num,
                                                  ContentType::Folders),
                                   HandleFolderGetContentsFolder);

                        ++yield_count;
                    }

                    // Yield this coroutine once if Files or Folders, twice if
                    // Both.
                    for (int i = 0; i < yield_count; ++i)
                        yield();

                    // Update the chunk number
                    if (folder_chunks_remaining_)
                        ++folders_chunk_num;

                    // Update the chunk number
                    if (file_chunks_remaining_)
                        ++files_chunk_num;
                }

                // Coroutine is done, so call the callback.
                callback_(files_, folders_, errors_);
            }};
};

}  // namespace mf
}  // namespace api

#include "get_folder_contents_impl.hpp"