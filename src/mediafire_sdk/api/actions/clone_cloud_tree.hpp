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

template <typename TRequest>
class CloneCloudTree : public Coroutine
{
    using FolderGetContentsRequestType = TRequest;

    using GetFolderContentsType
            = GetFolderContents<FolderGetContentsRequestType>;

public:
    using File = typename GetFolderContents<FolderGetContentsRequestType>::File;
    using Folder =
            typename GetFolderContents<FolderGetContentsRequestType>::Folder;

    using ErrorType =
            typename GetFolderContents<FolderGetContentsRequestType>::ErrorType;

    using CallbackType = std::function<void(const std::vector<File> &,
                                            const std::vector<Folder> &,
                                            const std::vector<ErrorType> &)>;

private:
    SessionMaintainer * stm_;
    std::shared_ptr<WorkManager> work_manager_;
    CallbackType callback_;

    std::vector<File> files_;
    std::vector<Folder> folders_;

    std::vector<ErrorType> errors_;

    std::deque<std::string> new_folder_keys_;

    push_type coro_{
            [this](pull_type & yield)
            {
                auto self = shared_from_this();  // Hold reference to ourselves
                                                 // until coroutine is complete
                while (!new_folder_keys_.empty())
                {
                    std::string folder_key = new_folder_keys_.front();
                    new_folder_keys_.pop_front();

                    typename GetFolderContentsType::CallbackType
                            HandleGetFolderContents = [this, self](
                                    const std::vector<File> & files,
                                    const std::vector<Folder> & folders,
                                    const std::vector<
                                            typename GetFolderContentsType::
                                                    ErrorType> & errors)
                    {
                        // Not much we can do with errors from
                        // folder/get_contents really, propagate them I guess
                        errors_.insert(std::end(errors_),
                                       std::begin(errors),
                                       std::end(errors));

                        // Append the results
                        files_.insert(std::end(files_),
                                      std::begin(files),
                                      std::end(files));
                        folders_.insert(std::end(folders_),
                                        std::begin(folders),
                                        std::end(folders));

                        for (const auto & folder : folders)
                        {
                            new_folder_keys_.push_back(folder.folderkey);
                        }

                        (*this)();
                    };

                    auto get_contents = GetFolderContentsType::Create(
                            stm_,
                            folder_key,
                            GetFolderContentsType::FilesOrFoldersOrBoth::Both,
                            std::move(HandleGetFolderContents));

                    work_manager_->QueueWork(get_contents, &yield);

                    if (new_folder_keys_.empty())
                        work_manager_->ExecuteWork();  // May insert more keys
                                                       // back onto
                                                       // new_folder_keys_
                                                       // continuing the loop
                }

                callback_(files_, folders_, errors_);
            }};

    /**
     *  @brief Private constructor.
     **/
    CloneCloudTree(SessionMaintainer * stm,
                   const std::string & folder_key,
                   std::shared_ptr<WorkManager> work_manager,
                   CallbackType && callback);

public:
    /**
     *  @brief  Public Create method.
     *
     *  @return shared_ptr holding newly created instance of class.
     **/
    static std::shared_ptr<CloneCloudTree> Create(
            SessionMaintainer * stm,
            const std::string & folder_key,
            std::shared_ptr<WorkManager> work_manager,
            CallbackType && call_back);

    /**
     *  @brief  Runs/resumes the coroutine
     **/
    void operator()() override;
};

}  // namespace mf
}  // namespace api

#include "clone_cloud_tree_impl.hpp"