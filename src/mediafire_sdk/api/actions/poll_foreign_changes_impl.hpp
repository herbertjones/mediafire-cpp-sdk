namespace mf
{

namespace api
{
template <typename TDeviceGetStatusRequest,
          typename TContactFetchRequest,
          typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
PollForeignChanges<TDeviceGetStatusRequest,
                   TContactFetchRequest,
                   TDeviceGetForeignChangesRequest,
                   TFolderGetInfoRequest,
                   TFileGetInfoRequest>::ContactFetchErrorType::
        ContactFetchErrorType(const std::error_code & error_code,
                              boost::optional<std::string> error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <typename TDeviceGetStatusRequest,
          typename TContactFetchRequest,
          typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
std::shared_ptr<PollForeignChanges<TDeviceGetStatusRequest,
                                   TContactFetchRequest,
                                   TDeviceGetForeignChangesRequest,
                                   TFolderGetInfoRequest,
                                   TFileGetInfoRequest>>
PollForeignChanges<TDeviceGetStatusRequest,
                   TContactFetchRequest,
                   TDeviceGetForeignChangesRequest,
                   TFolderGetInfoRequest,
                   TFileGetInfoRequest>::Create(SessionMaintainer * stm,
                                                uint32_t revision,
                                                std::shared_ptr<WorkManager>
                                                        work_manager,
                                                CallbackType && callback)
{
    return std::shared_ptr<PollForeignChanges>(new PollForeignChanges(
            stm, revision, work_manager, std::move(callback)));
}

template <typename TDeviceGetStatusRequest,
          typename TContactFetchRequest,
          typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
void PollForeignChanges<TDeviceGetStatusRequest,
                        TContactFetchRequest,
                        TDeviceGetForeignChangesRequest,
                        TFolderGetInfoRequest,
                        TFileGetInfoRequest>::
operator()()
{
    coro_();
}

template <typename TDeviceGetStatusRequest,
          typename TContactFetchRequest,
          typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
PollForeignChanges<TDeviceGetStatusRequest,
                   TContactFetchRequest,
                   TDeviceGetForeignChangesRequest,
                   TFolderGetInfoRequest,
                   TFileGetInfoRequest>::
        PollForeignChanges(SessionMaintainer * stm,
                           uint32_t revision,
                           std::shared_ptr<WorkManager> work_manager,
                           CallbackType && callback)
        : stm_(stm),
          revision_(revision),
          work_manager_(work_manager),
          callback_(callback)
{
}

}  // namespace mf
}  // namespace api