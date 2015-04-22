namespace mf
{

namespace api
{
template <typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
std::shared_ptr<PollForeignChanges<TDeviceGetForeignChangesRequest,
                                   TFolderGetInfoRequest,
                                   TFileGetInfoRequest>>
PollForeignChanges<TDeviceGetForeignChangesRequest,
                   TFolderGetInfoRequest,
                   TFileGetInfoRequest>::Create(SessionMaintainer * stm,
                                                const std::string & contact_key,
                                                uint32_t revision,
                                                std::shared_ptr<WorkManager>
                                                        work_manager,
                                                CallbackType && callback)
{
    return std::shared_ptr<PollForeignChanges>(new PollForeignChanges(
            stm, contact_key, revision, work_manager, std::move(callback)));
}

template <typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
PollForeignChanges<TDeviceGetForeignChangesRequest,
                   TFolderGetInfoRequest,
                   TFileGetInfoRequest>::
        PollForeignChanges(SessionMaintainer * stm,
                           const std::string & contact_key,
                           uint32_t revision,
                           std::shared_ptr<WorkManager> work_manager,
                           CallbackType && callback)
        : stm_(stm),
          contact_key_(contact_key),
          revision_(revision),
          work_manager_(work_manager),
          callback_(callback)
{
}

template <typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
void PollForeignChanges<TDeviceGetForeignChangesRequest,
                        TFolderGetInfoRequest,
                        TFileGetInfoRequest>::Cancel()
{
    cancelled_ = true;
}

template <typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
void PollForeignChanges<TDeviceGetForeignChangesRequest,
                        TFolderGetInfoRequest,
                        TFileGetInfoRequest>::
        HandleGetForeignChangesDevice(
                uint32_t latest_changes_revision,
                const std::vector<File> & updated_files,
                const std::vector<Folder> & updated_folders,
                const std::vector<File> & deleted_files,
                const std::vector<Folder> & deleted_folders,
                const std::vector<DeviceGetForeignChangesErrorType> &
                        get_changes_errors)
{
    latest_changes_revision_ = latest_changes_revision;
    get_changes_errors_ = get_changes_errors;

    updated_files_ = updated_files;
    updated_folders_ = updated_folders;
    deleted_files_ = deleted_files;
    deleted_folders_ = deleted_folders;

    // Resume
    Resume();
}

template <typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
void PollForeignChanges<TDeviceGetForeignChangesRequest,
                        TFolderGetInfoRequest,
                        TFileGetInfoRequest>::
        HandleGetInfoFile(const GetInfoFileResponseType & response,
                          const std::vector<FileGetInfoErrorType> & errors)
{
    if (response.error_code)
    {
        get_info_file_errors_.insert(std::end(get_info_file_errors_),
                                     std::begin(errors),
                                     std::end(errors));
    }
    else
    {
        updated_files_info_.push_back(response);
    }

    Resume();
}

template <typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
void PollForeignChanges<TDeviceGetForeignChangesRequest,
                        TFolderGetInfoRequest,
                        TFileGetInfoRequest>::
        HandleGetInfoFolder(const GetInfoFolderResponseType & response,
                            const std::vector<FolderGetInfoErrorType> & errors)
{
    if (response.error_code)
    {
        get_info_folder_errors_.insert(std::end(get_info_folder_errors_),
                                       std::begin(errors),
                                       std::end(errors));
    }
    else
    {
        updated_folders_info_.push_back(response);
    }

    Resume();
}

template <typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
void PollForeignChanges<TDeviceGetForeignChangesRequest,
                        TFolderGetInfoRequest,
                        TFileGetInfoRequest>::CoroutineBody(pull_type & yield)
{
    auto self = shared_from_this();  // Hold a reference to our
    // object until the coroutine
    // is complete, otherwise
    // handler will have invalid
    // reference to this because
    // the base object has
    // disappeared from scope

    //  1. For each contact key
    //      a) GetForeignChangesDevice to get changes for each
    //      contact key
    //      b) Call file/folder get_info for each of the updated
    //         files and folders

    auto HandleGetForeignChangesDevice =
            [this, self](uint32_t latest_changes_revision,
                         const std::vector<File> & updated_files,
                         const std::vector<Folder> & updated_folders,
                         const std::vector<File> & deleted_files,
                         const std::vector<Folder> & deleted_folders,
                         const std::vector<DeviceGetForeignChangesErrorType> &
                                 get_changes_errors)
    {
        this->HandleGetForeignChangesDevice(latest_changes_revision,
                                            updated_files,
                                            updated_folders,
                                            deleted_files,
                                            deleted_folders,
                                            get_changes_errors);
    };

    auto get_foreign_changes_device = GetForeignChangesDeviceType::Create(
            stm_,
            contact_key_,
            revision_,
            std::move(HandleGetForeignChangesDevice));

    if (!cancelled_)
    {
        get_foreign_changes_device->Start();
        yield();
    }

    // Get info on all the files
    for (const auto & file : updated_files_)
    {
        auto HandleGetInfoFile =
                [this, self](const GetInfoFileResponseType & response,
                             const std::vector<FileGetInfoErrorType> & errors)
        {
            this->HandleGetInfoFile(response, errors);
        };

        auto get_info_file = GetInfoFileType::Create(
                stm_, file.quickkey, std::move(HandleGetInfoFile));
        work_manager_->QueueWork(get_info_file, &yield);
    }

    if (cancelled_)
        work_manager_->Cancel();
    else
        work_manager_->ExecuteWork();

    // Get info on all the folders
    for (const auto & folder : updated_folders_)
    {
        auto HandleGetInfoFolder =
                [this, self](const GetInfoFolderResponseType & response,
                             const std::vector<FolderGetInfoErrorType> & errors)
        {
            this->HandleGetInfoFolder(response, errors);
        };

        // Call GetFolderInfo for each updated folder
        auto get_info_folder = GetInfoFolderType::Create(
                stm_, folder.folderkey, std::move(HandleGetInfoFolder));
        work_manager_->QueueWork(get_info_folder, &yield);
    }

    if (cancelled_)
        work_manager_->Cancel();
    else
        work_manager_->ExecuteWork();

    // Coroutine is done, so call the callback.
    callback_(latest_changes_revision_,
              deleted_files_,
              deleted_folders_,
              updated_files_info_,
              updated_folders_info_,
              get_changes_errors_,
              get_info_file_errors_,
              get_info_folder_errors_);
}

}  // namespace mf
}  // namespace api