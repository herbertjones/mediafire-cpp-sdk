#pragma once

#include <memory>

#include "work_manager.hpp"
#include "get_folder_contents.hpp"

#include "mediafire_sdk/api/session_maintainer.hpp"

#include "coroutine.hpp"

namespace
{
    int MAX_LOOP_ITERATIONS = 100;
}

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

    bool cancelled_ = false;

    std::vector<File> files_;
    std::vector<Folder> folders_;

    std::vector<ErrorType> errors_;

    std::deque<std::string> new_folder_keys_;

    void CoroutineBody(pull_type & yield) override;

    void Cancel();

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
};

}  // namespace mf
}  // namespace api

#include "clone_cloud_tree_impl.hpp"