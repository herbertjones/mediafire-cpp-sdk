#pragma once

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/device/get_foreign_resources.hpp"

#include "coroutine.hpp"

#include "boost/coroutine/all.hpp"

#include "get_foreign_changes_device.hpp"
#include "get_info_folder.hpp"
#include "get_info_file.hpp"
#include "work_manager.hpp"

namespace mf
{
namespace api
{

template <typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
class PollForeignChanges : public Coroutine
{
public:
    // Some convenience typedefs

    // device/get_changes
    using DeviceGetForeignChangesRequestType = TDeviceGetForeignChangesRequest;

    // folder/get_info
    using FolderGetInfoRequestType = TFolderGetInfoRequest;

    // file/get_info
    using FileGetInfoRequestType = TFileGetInfoRequest;

    // GetForeignChangesDevice
    using GetForeignChangesDeviceType
            = GetForeignChangesDevice<DeviceGetForeignChangesRequestType>;

    using File = typename GetForeignChangesDeviceType::File;
    using Folder = typename GetForeignChangesDeviceType::Folder;

    // GetInfoFolder
    using GetInfoFolderType = GetInfoFolder<FolderGetInfoRequestType>;
    using GetInfoFolderResponseType = typename GetInfoFolderType::ResponseType;

    // GetInfoFile
    using GetInfoFileType = GetInfoFile<FileGetInfoRequestType>;
    using GetInfoFileResponseType = typename GetInfoFileType::ResponseType;

    using FolderInfo = typename GetInfoFolderType::ResponseType;
    using FileInfo = typename GetInfoFileType::ResponseType;

    using DeviceGetForeignChangesErrorType =
            typename GetForeignChangesDeviceType::
                    DeviceGetForeignChangesErrorType;
    using FolderGetInfoErrorType = typename GetInfoFolderType::ErrorType;
    using FileGetInfoErrorType = typename GetInfoFileType::ErrorType;

    using CallbackType = std::function<void(
            uint32_t latest_changes_revision,
            const std::vector<File> & deleted_files,
            const std::vector<Folder> & deleted_folders,
            const std::vector<FileInfo> & updated_files_info,
            const std::vector<FolderInfo> & updated_folders_info,
            const std::vector<DeviceGetForeignChangesErrorType> &
                    get_changes_errors,
            const std::vector<FileGetInfoErrorType> & get_info_file_errors,
            const std::vector<FolderGetInfoErrorType> &
                    get_info_folder_errors)>;

public:
    /**
     *  @brief Create an instance and get us the shared pointer to the created
     *instance.
     *
     *  @return std::shared_ptr Shared pointer to the created instance.
     **/
    static std::shared_ptr<PollForeignChanges> Create(
            SessionMaintainer * stm,
            const std::string & contact_key,
            uint32_t revision,
            std::shared_ptr<WorkManager> work_manager,
            CallbackType && callback);

    void Cancel() override;

private:
    /**
     *  @brief  Private constructor.
     **/
    PollForeignChanges(SessionMaintainer * stm,
                       const std::string & contact_key,
                       uint32_t revision,
                       std::shared_ptr<WorkManager> work_manager,
                       CallbackType && callback);

    void HandleGetForeignChangesDevice(
            uint32_t latest_changes_revision,
            const std::vector<File> & updated_files,
            const std::vector<Folder> & updated_folders,
            const std::vector<File> & deleted_files,
            const std::vector<Folder> & deleted_folders,
            const std::vector<DeviceGetForeignChangesErrorType> &
                    get_changes_errors);

    void HandleGetInfoFile(const GetInfoFileResponseType & response,
                           const std::vector<FileGetInfoErrorType> & errors);

    void HandleGetInfoFolder(
            const GetInfoFolderResponseType & response,
            const std::vector<FolderGetInfoErrorType> & errors);

    void CoroutineBody(pull_type & yield) override;

private:
    SessionMaintainer * stm_;

    std::string contact_key_;
    uint32_t revision_;

    std::shared_ptr<WorkManager> work_manager_;

    CallbackType callback_;

    // Vectors to store intermediate results
    std::vector<File> updated_files_;
    std::vector<Folder> updated_folders_;

    // Vectors to hold callback data
    std::vector<File> deleted_files_;
    std::vector<Folder> deleted_folders_;

    std::vector<DeviceGetForeignChangesErrorType> get_changes_errors_;
    std::vector<FolderGetInfoErrorType> get_info_folder_errors_;
    std::vector<FileGetInfoErrorType> get_info_file_errors_;

    std::vector<GetInfoFolderResponseType> updated_folders_info_;
    std::vector<GetInfoFileResponseType> updated_files_info_;

    uint32_t latest_changes_revision_;

    bool cancelled_ = false;
};

}  // namespace mf
}  // namespace api

#include "poll_foreign_changes_impl.hpp"