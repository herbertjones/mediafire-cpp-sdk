namespace mf
{
namespace api
{

// template <class TGetChangesRequest>
// PollChanges<TGetChangesRequest>::ErrorType::ErrorType(
//        const std::error_code & error_code,
//        const std::string & error_string)
//        : error_code(error_code), error_string(error_string)
//{
//}

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
std::shared_ptr<PollChanges<TDeviceGetStatusRequest,
                            TDeviceGetChangesRequest,
                            TFolderGetInfoRequest,
                            TFileGetInfoRequest>>
PollChanges<TDeviceGetStatusRequest,
            TDeviceGetChangesRequest,
            TFolderGetInfoRequest,
            TFileGetInfoRequest>::Create(SessionMaintainer * stm,
                                         uint32_t revision,
                                         std::shared_ptr<WorkManager>
                                                 work_manager,
                                         CallbackType && callback)
{
    return std::shared_ptr<PollChanges>(
            new PollChanges(stm, revision, work_manager, std::move(callback)));
}

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
void PollChanges<TDeviceGetStatusRequest,
                 TDeviceGetChangesRequest,
                 TFolderGetInfoRequest,
                 TFileGetInfoRequest>::
operator()()
{
    coro_();
}

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
PollChanges<TDeviceGetStatusRequest,
            TDeviceGetChangesRequest,
            TFolderGetInfoRequest,
            TFileGetInfoRequest>::PollChanges(SessionMaintainer * stm,
                                              uint32_t revision,
                                              std::shared_ptr<WorkManager>
                                                      work_manager,
                                              CallbackType && callback)
        : stm_(stm),
          revision_(revision),
          work_manager_(work_manager),
          callback_(callback)
{
}
}  // namespace mf
}  // namespace api