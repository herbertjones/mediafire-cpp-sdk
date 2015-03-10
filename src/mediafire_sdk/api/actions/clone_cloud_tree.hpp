#pragma once

#include <memory>

#include "work_manager.hpp"
#include "get_folder_contents.hpp"

#include "mediafire_sdk/api/session_maintainer.hpp"

#include "coroutine.hpp"

namespace mf
{
namespace api
{

class CloneCloudTree : public Coroutine
{
public:
    using File = GetFolderContents::File;
    using Folder = GetFolderContents::Folder;

    using CallbackType = std::function<void(const std::vector<File> &, const std::vector<Folder> &)>;

private:
    SessionMaintainer * stm_;

    CallbackType callback_;

    std::vector<File> files_;
    std::vector<Folder> folders_;

    std::deque<std::string> new_folder_keys_;

    push_type coro_
    {
        [this](pull_type & yield)
        {
            auto ref = shared_from_this(); // Hold reference to ourselves until coroutine is complete

            WorkManager<std::shared_ptr<GetFolderContents>> work_manager(stm_->HttpConfig()->GetDefaultCallbackIoService()); // Use this to avoid shoving too much work into io_service
            work_manager.SetMaxConcurrentWork(10);

            int num_queued = 0;
            while (true)
            {
                if (!new_folder_keys_.empty())
                {
                    // Queue some work

                    std::string folder_key = new_folder_keys_.front();
                    new_folder_keys_.pop_front();

                    // Queue the folders
                    GetFolderContents::CallbackType HandleGetFolderContentsFolders =
                    [this](const std::vector<File> & files, const std::vector<Folder> & folders)
                    {
                        folders_.insert(std::end(folders_), std::begin(folders), std::end(folders));

                        for (const auto & folder : folders)
                        {
                            new_folder_keys_.push_back(folder.folderkey);
                        }

                        (*this)();
                    };

                    std::shared_ptr<GetFolderContents> get_contents_folders = GetFolderContents::Create(stm_, folder_key, GetFolderContents::ContentType::Folders, std::move(HandleGetFolderContentsFolders));

                    work_manager.QueueWork(get_contents_folders);

                    // Queue the files
                    GetFolderContents::CallbackType HandleGetFolderContentsFiles =
                    [this](const std::vector<File> & files, const std::vector<Folder> & folders)
                    {
                        files_.insert(std::end(files_), std::begin(files), std::end(files));

                        (*this)();
                    };

                    std::shared_ptr<GetFolderContents> get_contents_files = GetFolderContents::Create(stm_, folder_key, GetFolderContents::ContentType::Files, std::move(HandleGetFolderContentsFiles));
                    
                    work_manager.QueueWork(get_contents_files);
                    
                    num_queued += 2;
                }
                else
                {
                    // No work to queue

                    if (num_queued > 0)
                    {
                        yield();
                        // Handler returned
                        --num_queued;
                    }
                    else
                    {
                        break; // All handlers we queued returned
                    }
                }
            }

            callback_(files_, folders_);
        }
    };


    CloneCloudTree(SessionMaintainer * stm, const std::string & folder_key, CallbackType && callback) : stm_(stm), callback_(std::move(callback)) { new_folder_keys_.push_back(folder_key); }

public:
    static std::shared_ptr<CloneCloudTree> Create(SessionMaintainer * stm, const std::string & folder_key, CallbackType && call_back)
    {
        return std::shared_ptr<CloneCloudTree>(new CloneCloudTree(stm, folder_key, std::move(call_back)));
    }

    void operator()() override { coro_(); }
};

} // namespace mf
} // namespace api