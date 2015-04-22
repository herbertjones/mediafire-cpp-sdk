namespace mf
{
namespace api
{

template <typename TDeviceGetForeignResourcesRequest>
GetForeignResourcesDevice<TDeviceGetForeignResourcesRequest>::ErrorType::
        ErrorType(const std::error_code & error_code,
                  const boost::optional<std::string> & error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <typename TDeviceGetForeignResourcesRequest>
std::shared_ptr<GetForeignResourcesDevice<TDeviceGetForeignResourcesRequest>>
GetForeignResourcesDevice<TDeviceGetForeignResourcesRequest>::Create(
        SessionMaintainer * stm,
        CallbackType && callback)
{
    return std::shared_ptr<GetForeignResourcesDevice>(
            new GetForeignResourcesDevice(stm, std::move(callback)));
}

template <typename TDeviceGetForeignResourcesRequest>
GetForeignResourcesDevice<TDeviceGetForeignResourcesRequest>::
        GetForeignResourcesDevice(SessionMaintainer * stm,
                                  CallbackType && callback)
        : stm_(stm), callback_(callback)
{
}

template <typename TDeviceGetForeignResourcesRequest>
void GetForeignResourcesDevice<TDeviceGetForeignResourcesRequest>::Cancel()
{
}

template <typename TDeviceGetForeignResourcesRequest>
void GetForeignResourcesDevice<TDeviceGetForeignResourcesRequest>::
        CoroutineBody(pull_type & yield)
{
    auto self = shared_from_this();  // Hold a reference to our
    // object until the coroutine
    // is complete, otherwise
    // handler will have invalid
    // reference to this because
    // the base object has
    // disappeared from scope

    auto HandleDeviceGetForeignResources =
            [this, self](const DeviceGetForeignResourcesResponseType & response)
    {
        if (response.error_code)
        {
            errors_.push_back(
                    ErrorType(response.error_code, response.error_string));
        }
        else
        {
            files_ = response.files;
            folders_ = response.folders;
        }

        Resume();
    };

    stm_->Call(DeviceGetForeignResourcesRequestType(),
               HandleDeviceGetForeignResources);

    yield();

    callback_(files_, folders_, errors_);
}
}  // namespace mf
}  // namespace api