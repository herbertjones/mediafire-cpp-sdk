#pragma once

#include "mediafire_sdk/api/session_maintainer.hpp"

#include "coroutine.hpp"

#include "boost/coroutine/all.hpp"

namespace mf
{
namespace api
{

template <typename TDeviceGetStatusRequest, typename TDeviceGetChangesRequest>
class GetChangesDevice : public Coroutine
{
public:
    // Some convenience typedefss

    // device/get_status
    using DeviceGetStatusRequestType = TDeviceGetStatusRequest;
    using DeviceGetStatusResponseType =
            typename DeviceGetStatusRequestType::ResponseType;

    // device/get_changes
    using DeviceGetChangesRequestType = TDeviceGetChangesRequest;
    using DeviceGetChangesResponseType =
            typename DeviceGetChangesRequestType::ResponseType;

    using File = typename DeviceGetChangesResponseType::File;
    using Folder = typename DeviceGetChangesResponseType::Folder;

    // The struct for the errors we might return
    struct DeviceGetStatusErrorType
    {
        DeviceGetStatusErrorType(const std::error_code & error_code,
                                 const std::string & error_string);

        std::error_code error_code;
        std::string error_string;
    };

    // The struct for the errors we might returnu
    struct DeviceGetChangesErrorType
    {
        DeviceGetChangesErrorType(uint32_t start_revision,
                                  const std::error_code & error_code,
                                  const std::string & error_string);

        uint32_t start_revision;
        std::error_code error_code;
        std::string error_string;
    };

    using CallbackType = std::function<void(
            uint32_t latest_device_revision,
            const std::vector<File> & updated_files,
            const std::vector<Folder> & updated_folders,
            const std::vector<File> & deleted_files,
            const std::vector<Folder> & deleted_folders,
            const std::vector<DeviceGetStatusErrorType> & get_status_errors,
            const std::vector<DeviceGetChangesErrorType> & get_changes_errors)>;

public:
    /**
     *  @brief Create an instance and get us the shared pointer to the created
     *instance.
     *
     *  @return std::shared_ptr Shared pointer to the created instance.
     **/
    static std::shared_ptr<GetChangesDevice> Create(
            SessionMaintainer * stm,
            uint32_t latest_known_revision,
            CallbackType && callback);

    /**
     *  @brief Starts/resumes the coroutine.
     */
    void operator()() override;

private:
    /**
     *  @brief  Private constructor.
     **/
    GetChangesDevice(SessionMaintainer * stm,
                     uint32_t latest_known_revision,
                     CallbackType && callback);

private:
    SessionMaintainer * stm_;

    uint32_t latest_known_revision_;
    uint32_t latest_device_revision_;

    CallbackType callback_;

    std::vector<DeviceGetStatusErrorType> get_status_errors_;
    std::vector<DeviceGetChangesErrorType> get_changes_errors_;

    // Vectors to hold callback data
    std::vector<File> updated_files_;
    std::vector<Folder> updated_folders_;
    std::vector<File> deleted_files_;
    std::vector<Folder> deleted_folders_;

    push_type coro_{
            [this](pull_type & yield)
            {
                auto self = shared_from_this(); // Hold a reference to our
                                                // object until the coroutine
                                                // is complete, otherwise
                                                // handler will have invalid
                                                // reference to this because
                                                // the base object has
                                                // disappeared from scope

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

                uint32_t start_revision = latest_known_revision_;
                uint32_t min_revision = latest_known_revision_ + 1;
                uint32_t max_revision = ((min_revision / 500) + 1) * 500;

                while (start_revision < latest_device_revision_)
                {
                    std::function<void(const DeviceGetChangesResponseType &
                                               response)> HandleDeviceGetChanges
                            = [this, self, start_revision](
                                    const DeviceGetChangesResponseType &
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

                            get_changes_errors_.push_back(
                                    DeviceGetChangesErrorType(
                                            start_revision,
                                            response.error_code,
                                            error_str));
                        }
                        else
                        {
                            updated_files_.insert(
                                    std::end(updated_files_),
                                    std::begin(response.updated_files),
                                    std::end(response.updated_files));
                            updated_folders_.insert(
                                    std::end(updated_folders_),
                                    std::begin(response.updated_folders),
                                    std::end(response.updated_folders));
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

                    stm_->Call(DeviceGetChangesRequestType(start_revision),
                               HandleDeviceGetChanges);

                    yield();

                    start_revision = max_revision;
                    min_revision = start_revision + 1;
                    max_revision = ((min_revision / 500) + 1) * 500;
                }

                // Coroutine is done, so call the callback.
                callback_(latest_device_revision_,
                          updated_files_,
                          updated_folders_,
                          deleted_files_,
                          deleted_folders_,
                          get_status_errors_,
                          get_changes_errors_);
            }};
};

}  // namespace mf
}  // namespace api

#include "get_changes_device_impl.hpp"