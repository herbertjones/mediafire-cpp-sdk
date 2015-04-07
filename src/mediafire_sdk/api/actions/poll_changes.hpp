#pragma once

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/device/get_status.hpp"

#include "coroutine.hpp"

#include "boost/coroutine/all.hpp"

#include "get_changes_device.hpp"
#include "get_info_folder.hpp"
#include "get_info_file.hpp"
#include "work_manager.hpp"

namespace mf
{
namespace api
{

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
class PollChanges : public Coroutine
{
public:
    // Some convenience typedefs

    // device/get_status
    using DeviceGetStatusRequestType = TDeviceGetStatusRequest;

    // device/get_changes
    using DeviceGetChangesRequestType = TDeviceGetChangesRequest;

    // folder/get_info
    using FolderGetInfoRequestType = TFolderGetInfoRequest;

    // file/get_info
    using FileGetInfoRequestType = TFileGetInfoRequest;

    // GetChangesDevice
    using GetChangesDeviceType = GetChangesDevice<DeviceGetStatusRequestType,
                                                  DeviceGetChangesRequestType>;

    using File = typename GetChangesDeviceType::File;
    using Folder = typename GetChangesDeviceType::Folder;

    // GetInfoFolder
    using GetInfoFolderType = GetInfoFolder<FolderGetInfoRequestType>;
    using GetInfoFolderResponseType = typename GetInfoFolderType::ResponseType;

    // GetInfoFile
    using GetInfoFileType = GetInfoFile<FileGetInfoRequestType>;
    using GetInfoFileResponseType = typename GetInfoFileType::ResponseType;

    using FolderInfo = typename GetInfoFolderType::ResponseType;
    using FileInfo = typename GetInfoFileType::ResponseType;

    // Error types
    using DeviceGetStatusErrorType =
            typename GetChangesDeviceType::DeviceGetStatusErrorType;
    using DeviceGetChangesErrorType =
            typename GetChangesDeviceType::DeviceGetChangesErrorType;
    using GetInfoFolderErrorType = typename GetInfoFolderType::ErrorType;
    using GetInfoFileErrorType = typename GetInfoFileType::ErrorType;

    using CallbackType = std::function<void(
            uint32_t latest_device_revison,
            const std::vector<File> & deleted_files,
            const std::vector<Folder> & deleted_folders,
            const std::vector<FileInfo> & updated_files_info,
            const std::vector<FolderInfo> & updated_folders_info,
            const std::vector<DeviceGetStatusErrorType> & get_status_errors,
            const std::vector<DeviceGetChangesErrorType> & get_changes_errors,
            const std::vector<GetInfoFileErrorType> & get_info_file_errors,
            const std::vector<GetInfoFolderErrorType> &
                    get_info_folder_errors)>;

public:
    /**
     *  @brief Create an instance and get us the shared pointer to the created
     *instance.
     *
     *  @return std::shared_ptr Shared pointer to the created instance.
     **/
    static std::shared_ptr<PollChanges> Create(
            SessionMaintainer * stm,
            uint32_t revision,
            std::shared_ptr<WorkManager> work_manager,
            CallbackType && callback);

    /**
     *  @brief Starts/resumes the coroutine.
     */
    void operator()() override;

private:
    /**
     *  @brief  Private constructor.
     **/
    PollChanges(SessionMaintainer * stm,
                uint32_t revision,
                std::shared_ptr<WorkManager> work_manager,
                CallbackType && callback);

private:
    SessionMaintainer * stm_;

    uint32_t revision_;

    std::shared_ptr<WorkManager> work_manager_;

    CallbackType callback_;

    uint32_t latest_device_revision_ = 0;

    // Vectors to hold callback data
    std::vector<File> updated_files_;
    std::vector<Folder> updated_folders_;
    std::vector<File> deleted_files_;
    std::vector<Folder> deleted_folders_;

    std::vector<DeviceGetStatusErrorType> get_status_errors_;
    std::vector<DeviceGetChangesErrorType> get_changes_errors_;
    std::vector<GetInfoFolderErrorType> get_info_folder_errors_;
    std::vector<GetInfoFileErrorType> get_info_file_errors_;

    std::vector<GetInfoFolderResponseType> updated_folders_info_;
    std::vector<GetInfoFileResponseType> updated_files_info_;

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

                auto callback = [this, self](
                        uint32_t latest_device_revision,
                        const std::vector<File> & updated_files,
                        const std::vector<Folder> & updated_folders,
                        const std::vector<File> & deleted_files,
                        const std::vector<Folder> & deleted_folders,
                        const std::vector<DeviceGetStatusErrorType> &
                                get_status_errors,
                        const std::vector<DeviceGetChangesErrorType> &
                                get_changes_errors)
                {
                    latest_device_revision_ = latest_device_revision;

                    get_status_errors_ = get_status_errors;
                    get_changes_errors_ = get_changes_errors;

                    updated_files_ = updated_files;

                    // updated_folders_ = updated_folders;
                    // Hack around updated_folders containing trash folder key
                    // -_-

                    for (const auto & updated_folder : updated_folders)
                        if (updated_folder.folderkey != "trash")
                            updated_folders_.push_back(updated_folder);

                    deleted_files_ = deleted_files;
                    deleted_folders_ = deleted_folders;

                    // Resume
                    (*this)();
                };

                auto get_changes_device = GetChangesDeviceType::Create(
                        stm_, revision_, std::move(callback));
                work_manager_->QueueWork(get_changes_device, &yield);
                work_manager_->ExecuteWork();

                // Get info on all the files
                for (const auto & file : updated_files_)
                {
                    auto callback = [this, self](
                            const GetInfoFileResponseType & response,
                            const std::vector<GetInfoFileErrorType> & errors)
                    {
                        if (!errors.empty())
                        {
                            get_info_file_errors_.insert(
                                    std::end(get_info_file_errors_),
                                    std::begin(errors),
                                    std::end(errors));
                        }
                        else
                        {
                            updated_files_info_.push_back(response);
                        }

                        (*this)();
                    };

                    auto get_info_file = GetInfoFileType::Create(
                            stm_, file.quickkey, std::move(callback));
                    work_manager_->QueueWork(get_info_file, &yield);
                }

                work_manager_->ExecuteWork();

                // Get info on all the folders
                for (const auto & folder : updated_folders_)
                {
                    auto callback = [this, self](
                            const GetInfoFolderResponseType & response,
                            const std::vector<GetInfoFolderErrorType> & errors)
                    {
                        if (!errors.empty())
                        {
                            get_info_folder_errors_.insert(
                                    std::end(get_info_folder_errors_),
                                    std::begin(errors),
                                    std::end(errors));
                        }
                        else
                        {
                            updated_folders_info_.push_back(response);
                        }

                        (*this)();
                    };

                    auto get_info_folder = GetInfoFolderType::Create(
                            stm_, folder.folderkey, std::move(callback));
                    work_manager_->QueueWork(get_info_folder, &yield);
                }

                work_manager_->ExecuteWork();

                // Coroutine is done, so call the callback.
                callback_(latest_device_revision_,
                          deleted_files_,
                          deleted_folders_,
                          updated_files_info_,
                          updated_folders_info_,
                          get_status_errors_,
                          get_changes_errors_,
                          get_info_file_errors_,
                          get_info_folder_errors_);
            }};
};

}  // namespace mf
}  // namespace api

#include "poll_changes_impl.hpp"