#pragma once

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/device/get_foreign_changes.hpp"

#include "coroutine.hpp"

#include "boost/coroutine/all.hpp"

namespace mf
{
namespace api
{

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetForeignChangesRequest>
class GetForeignChangesDevice : public Coroutine
{
public:
    // Some convenience typedefss

    // device/get_status
    using DeviceGetStatusRequestType = TDeviceGetStatusRequest;
    using DeviceGetStatusResponseType =
            typename DeviceGetStatusRequestType::ResponseType;

    // device/get_changes
    using DeviceGetForeignChangesRequestType = TDeviceGetForeignChangesRequest;
    using DeviceGetForeignChangesResponseType =
            typename DeviceGetForeignChangesRequestType::ResponseType;

    using File = typename DeviceGetForeignChangesResponseType::File;
    using Folder = typename DeviceGetForeignChangesResponseType::Folder;

    // The struct for the errors we might return
    struct DeviceGetStatusErrorType
    {
        DeviceGetStatusErrorType(const std::error_code & error_code,
                                 const std::string & error_string);

        std::error_code error_code;
        std::string error_string;
    };

    // The struct for the errors we might returnu
    struct DeviceGetForeignChangesErrorType
    {
        DeviceGetForeignChangesErrorType(uint32_t start_revision,
                                         const std::string & contact_key,
                                         const std::error_code & error_code,
                                         const std::string & error_string);

        uint32_t start_revision;
        std::string contact_key;
        std::error_code error_code;
        std::string error_string;
    };

    using CallbackType = std::function<void(
            const std::vector<File> & updated_files,
            const std::vector<Folder> & updated_folders,
            const std::vector<File> & deleted_files,
            const std::vector<Folder> & deleted_folders,
            const std::vector<DeviceGetStatusErrorType> & get_status_errors,
            const std::vector<DeviceGetForeignChangesErrorType> &
                    get_changes_errors)>;

public:
    /**
     *  @brief Create an instance and get us the shared pointer to the created
     *instance.
     *
     *  @return std::shared_ptr Shared pointer to the created instance.
     **/
    static std::shared_ptr<GetForeignChangesDevice> Create(
            SessionMaintainer * stm,
            uint32_t latest_known_revision,
            const std::string & contact_key,
            CallbackType && callback);

    /**
     *  @brief Starts/resumes the coroutine.
     */
    void operator()() override;

private:
    /**
     *  @brief  Private constructor.
     **/
    GetForeignChangesDevice(SessionMaintainer * stm,
                            uint32_t latest_known_revision,
                            const std::string & contact_key,
                            CallbackType && callback);

private:
    SessionMaintainer * stm_;

    uint32_t latest_known_revision_;

    std::string contact_key_;

    uint32_t latest_device_revision_;

    CallbackType callback_;

    std::vector<DeviceGetStatusErrorType> get_status_errors_;
    std::vector<DeviceGetForeignChangesErrorType> get_foreign_changes_errors_;

    // Vectors to hold callback data
    std::vector<File> updated_files_;
    std::vector<Folder> updated_folders_;
    std::vector<File> deleted_files_;
    std::vector<Folder> deleted_folders_;

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


                // device/get_status
                std::function<void(const DeviceGetStatusResponseType &
                                           response)> HandleDeviceGetStatus =
                        [this, self](
                                const DeviceGetStatusResponseType & response)
                {
                    if (response.error_code)
                    {
                        // If there was an error, insert into vector and
                        // propagate at the callback.
                        std::string error_str = "No error string provided";
                        if (response.error_string)
                            error_str = *response.error_string;

                        get_status_errors_.push_back(DeviceGetStatusErrorType(
                                response.error_code, error_str));
                    }
                    else
                    {
                        latest_device_revision_ = response.device_revision;
                    }

                    // Resume the coroutine
                    (*this)();
                };

                stm_->Call(DeviceGetStatusRequestType(), HandleDeviceGetStatus);

                yield();

                // device/get_foreign_changes
                uint32_t start_revision = latest_known_revision_;
                uint32_t min_revision = latest_known_revision_ + 1;
                uint32_t max_revision = ((min_revision / 500) + 1) * 500;

                while (start_revision < latest_device_revision_)
                {
                    std::function<void(
                            const DeviceGetForeignChangesResponseType &
                                    response)> HandleDeviceGetForeignChanges =
                            [this, self, start_revision](
                                    const DeviceGetForeignChangesResponseType &
                                            response)
                    {
                        if (response.error_code)
                        {
                            // If there was an error, insert
                            // into vector and
                            // propagate at the callback.
                            std::string error_str = "No error string provided ";
                            if (response.error_string)
                                error_str = *response.error_string;

                            get_foreign_changes_errors_.push_back(
                                    DeviceGetForeignChangesErrorType(
                                            start_revision,
                                            contact_key_,
                                            response.error_code,
                                            error_str));
                        }
                        else
                        {
//                            for (const auto & folder : response.updated_folders)
//                            {
//                                std::cout << folder.folderkey << " " << start_revision << " " << contact_key_ << std::endl;
//                            }

                            updated_files_.insert(
                                    std::end(updated_files_),
                                    std::begin(response.updated_files),
                                    std::end(response.updated_files));
//                            updated_folders_.insert(
//                                    std::end(updated_folders_),
//                                    std::begin(response.updated_folders),
//                                    std::end(response.updated_folders));

                            // HACK around the API including "trash" inside updated_folders
                            for (const auto & folder : response.updated_folders)
                                if (folder.folderkey != "trash")
                                    updated_folders_.push_back(folder);

                            deleted_files_.insert(
                                    std::end(deleted_files_),
                                    std::begin(response.deleted_files),
                                    std::end(response.deleted_files));
                            deleted_folders_.insert(
                                    std::end(deleted_folders_),
                                    std::begin(response.deleted_folders),
                                    std::end(response.deleted_folders));
                        }

                        // Resume the coroutine
                        (*this)();
                    };

                    stm_->Call(DeviceGetForeignChangesRequestType(
                                       start_revision, contact_key_),
                               HandleDeviceGetForeignChanges);

                    yield();

                    start_revision = max_revision;
                    min_revision = start_revision + 1;
                    max_revision = ((min_revision / 500) + 1) * 500;
                }

                // Coroutine is done, so call the callback.
                callback_(updated_files_,
                          updated_folders_,
                          deleted_files_,
                          deleted_folders_,
                          get_status_errors_,
                          get_foreign_changes_errors_);
            }};
};

}  // namespace mf
}  // namespace api

#include "get_foreign_changes_device_impl.hpp"