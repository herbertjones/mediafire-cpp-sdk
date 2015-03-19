namespace mf
{
namespace api
{

template <typename TDeviceGetStatusRequest, typename TDeviceGetChangesRequest>
GetChangesDevice<TDeviceGetStatusRequest, TDeviceGetChangesRequest>::
        DeviceGetStatusErrorType::DeviceGetStatusErrorType(
                const std::error_code & error_code,
                const std::string & error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <typename TDeviceGetStatusRequest, typename TDeviceGetChangesRequest>
GetChangesDevice<TDeviceGetStatusRequest, TDeviceGetChangesRequest>::
        DeviceGetChangesErrorType::DeviceGetChangesErrorType(
                uint32_t start_revision,
                const std::error_code & error_code,
                const std::string & error_string)
        : start_revision(start_revision),
          error_code(error_code),
          error_string(error_string)
{
}

template <typename TDeviceGetStatusRequest, typename TDeviceGetChangesRequest>
std::shared_ptr<GetChangesDevice<TDeviceGetStatusRequest,
                                 TDeviceGetChangesRequest>>
GetChangesDevice<TDeviceGetStatusRequest, TDeviceGetChangesRequest>::Create(
        SessionMaintainer * stm,
        uint32_t latest_known_revision,
        CallbackType && callback)
{
    return std::shared_ptr<GetChangesDevice>(new GetChangesDevice(
            stm, latest_known_revision, std::move(callback)));
}

template <typename TDeviceGetStatusRequest, typename TDeviceGetChangesRequest>
void GetChangesDevice<TDeviceGetStatusRequest, TDeviceGetChangesRequest>::
operator()()
{
    coro_();
}

template <typename TDeviceGetStatusRequest, typename TDeviceGetChangesRequest>
GetChangesDevice<TDeviceGetStatusRequest, TDeviceGetChangesRequest>::
        GetChangesDevice(SessionMaintainer * stm,
                         uint32_t latest_known_revision,
                         CallbackType && callback)
        : stm_(stm),
          latest_known_revision_(latest_known_revision),
          callback_(callback)
{
}
}  // namespace mf
}  // namespace api