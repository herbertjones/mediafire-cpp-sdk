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

template <typename TDeviceGetStatusRequest,
          typename TContactFetchRequest,
          typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
class PollForeignChanges : public Coroutine
{
public:
    // Some convenience typedefs

    // device/get_status
    using DeviceGetStatusRequestType = TDeviceGetStatusRequest;

    // contact/fetch
    using ContactFetchRequestType = TContactFetchRequest;
    using ContactFetchResponseType =
            typename ContactFetchRequestType::ResponseType;

    // device/get_changes
    using DeviceGetForeignChangesRequestType = TDeviceGetForeignChangesRequest;

    // folder/get_info
    using FolderGetInfoRequestType = TFolderGetInfoRequest;

    // file/get_info
    using FileGetInfoRequestType = TFileGetInfoRequest;

    // GetForeignChangesDevice
    using GetForeignChangesDeviceType
            = GetForeignChangesDevice<DeviceGetStatusRequestType,
                                      DeviceGetForeignChangesRequestType>;

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

    // Error types
    struct ContactFetchErrorType
    {
        ContactFetchErrorType(const std::error_code & error_code,
                              boost::optional<std::string> error_string);

        std::error_code error_code;
        boost::optional<std::string> error_string;
    };

    using DeviceGetStatusErrorType =
            typename GetForeignChangesDeviceType::DeviceGetStatusErrorType;
    using DeviceGetForeignChangesErrorType =
            typename GetForeignChangesDeviceType::
                    DeviceGetForeignChangesErrorType;
    using GetInfoFolderErrorType = typename GetInfoFolderType::ErrorType;
    using GetInfoFileErrorType = typename GetInfoFileType::ErrorType;

    using CallbackType = std::function<void(
            const std::vector<File> & deleted_files,
            const std::vector<Folder> & deleted_folders,
            const std::vector<FileInfo> & updated_files_info,
            const std::vector<FolderInfo> & updated_folders_info,
            const std::vector<DeviceGetStatusErrorType> & get_status_errors,
            const std::vector<ContactFetchErrorType> & contact_fetch_errors,
            const std::vector<DeviceGetForeignChangesErrorType> &
                    get_changes_errors,
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
    static std::shared_ptr<PollForeignChanges> Create(
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
    PollForeignChanges(SessionMaintainer * stm,
                       uint32_t revision,
                       std::shared_ptr<WorkManager> work_manager,
                       CallbackType && callback);

private:
    SessionMaintainer * stm_;

    uint32_t revision_;

    std::shared_ptr<WorkManager> work_manager_;

    CallbackType callback_;

    // Vectors to store intermediate results
    std::vector<File> updated_files_;
    std::vector<Folder> updated_folders_;
    std::vector<std::string> contact_keys_;

    // Vectors to hold callback data
    std::vector<File> deleted_files_;
    std::vector<Folder> deleted_folders_;

    std::vector<DeviceGetStatusErrorType> get_status_errors_;
    std::vector<ContactFetchErrorType> contact_fetch_errors_;
    std::vector<DeviceGetForeignChangesErrorType> get_changes_errors_;
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

                //  1. contact/fetch to get list of contact keys
                //  2. For each contact key
                //      a) GetForeignChangesDevice to get changes for each
                //      contact key
                //      b) Call file/folder get_info for each of the updated
                //         files and folders

                auto HandleContactFetch =
                        [this, self](const ContactFetchResponseType & response)
                {
                    if (response.error_code)
                    {
                        contact_fetch_errors_.push_back(ContactFetchErrorType(
                                response.error_code, response.error_string));
                    }
                    else
                    {
                        for (const auto & contact : response.contacts)
                            contact_keys_.push_back(contact.contact_key);
                    }
                    (*this)();
                };

                stm_->Call(ContactFetchRequestType(), HandleContactFetch);

                yield();

                for (const auto & contact_key : contact_keys_)
                {
                    auto HandleGetForeignChangesDevice = [this, self](
                            const std::vector<File> & updated_files,
                            const std::vector<Folder> & updated_folders,
                            const std::vector<File> & deleted_files,
                            const std::vector<Folder> & deleted_folders,
                            const std::vector<DeviceGetStatusErrorType> &
                                    get_status_errors,
                            const std::
                                    vector<DeviceGetForeignChangesErrorType> &
                                            get_changes_errors)
                    {
                        get_status_errors_ = get_status_errors;
                        get_changes_errors_ = get_changes_errors;

                        updated_files_ = updated_files;
                        updated_folders_ = updated_folders;
                        deleted_files_ = deleted_files;
                        deleted_folders_ = deleted_folders;

                        // Resume
                        (*this)();
                    };

                    auto get_foreign_changes_device
                            = GetForeignChangesDeviceType::Create(
                                    stm_,
                                    revision_,
                                    contact_key,
                                    std::move(HandleGetForeignChangesDevice));
                    work_manager_->QueueWork(get_foreign_changes_device,
                                             &yield);
                    work_manager_->ExecuteWork();

                    // Get info on all the files
                    for (const auto & file : updated_files_)
                    {
                        auto callback = [this, self](
                                const GetInfoFileResponseType & response,
                                const std::vector<GetInfoFileErrorType> &
                                        errors)
                        {
                            if (response.error_code)
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
                                const std::vector<GetInfoFolderErrorType> &
                                        errors)
                        {
                            if (response.error_code)
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

                        // Call GetFolderInfo for each updated folder
                        auto get_info_folder = GetInfoFolderType::Create(
                                stm_, folder.folderkey, std::move(callback));
                        work_manager_->QueueWork(get_info_folder, &yield);
                    }

                    work_manager_->ExecuteWork();
                }

                // Coroutine is done, so call the callback.
                callback_(deleted_files_,
                          deleted_folders_,
                          updated_files_info_,
                          updated_folders_info_,
                          get_status_errors_,
                          contact_fetch_errors_,
                          get_changes_errors_,
                          get_info_file_errors_,
                          get_info_folder_errors_);
            }};
};

}  // namespace mf
}  // namespace api

#include "poll_foreign_changes_impl.hpp"