namespace
{
    int MAX_LOOP_ITERATIONS = 100;
}

namespace mf
{
namespace api
{

template <typename TRequest>
CloneCloudTree<TRequest>::CloneCloudTree(
        SessionMaintainer * stm,
        const std::string & folder_key,
        std::shared_ptr<WorkManager> work_manager,
        CallbackType && callback)
        : stm_(stm), work_manager_(work_manager), callback_(std::move(callback))
{
    new_folder_keys_.push_back(folder_key);
}

template <typename TRequest>
std::shared_ptr<CloneCloudTree<TRequest>> CloneCloudTree<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & folder_key,
        std::shared_ptr<WorkManager> work_manager,
        CallbackType && callback)
{
    return std::shared_ptr<CloneCloudTree>(new CloneCloudTree(
            stm, folder_key, work_manager, std::move(callback)));
}

template <typename TRequest>
void CloneCloudTree<TRequest>::HandleGetFolderContents(
        const std::vector<File> & files,
        const std::vector<Folder> & folders,
        const std::vector<typename GetFolderContentsType::ErrorType> & errors)
{
    // Not much we can do with errors from
    // folder/get_contents really, propagate them I guess
    errors_.insert(std::end(errors_), std::begin(errors), std::end(errors));

    // Append the results
    files_.insert(std::end(files_), std::begin(files), std::end(files));
    folders_.insert(std::end(folders_), std::begin(folders), std::end(folders));

    for (const auto & folder : folders)
    {
        new_folder_keys_.push_back(folder.folderkey);
    }

    Resume();
}

template <typename TRequest>
void CloneCloudTree<TRequest>::CoroutineBody(pull_type & yield)
{
    auto self = shared_from_this();  // Hold reference to ourselves
    // until coroutine is complete
    int num_loop_iterations = 0;
    while (!new_folder_keys_.empty())
    {
        if (num_loop_iterations > MAX_LOOP_ITERATIONS)
        {
            assert(0);
            break;
        }

        std::string folder_key = new_folder_keys_.front();
        new_folder_keys_.pop_front();

        typename GetFolderContentsType::CallbackType HandleGetFolderContents =
                [this, self](
                        const std::vector<File> & files,
                        const std::vector<Folder> & folders,
                        const std::vector<
                                typename GetFolderContentsType::ErrorType> &
                                errors)
        {
            this->HandleGetFolderContents(files, folders, errors);
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

        ++num_loop_iterations;
    }

    callback_(files_, folders_, errors_);
}

template <typename TRequest>
void CloneCloudTree<TRequest>::Cancel()
{
    cancelled_ = true;

    work_manager_->Cancel();
}

}  // namespace api
}  // namespace mf