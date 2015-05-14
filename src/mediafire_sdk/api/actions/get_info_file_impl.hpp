namespace mf
{
namespace api
{

template <class TRequest>
GetInfoFile<TRequest>::ErrorType::ErrorType(const std::string & folder_key,
                                            const std::error_code & error_code,
                                            const std::string & error_string)
        : folder_key(folder_key),
          error_code(error_code),
          error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<GetInfoFile<TRequest>> GetInfoFile<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & folder_key,
        CallbackType && callback)
{
    return std::shared_ptr<GetInfoFile>(
            new GetInfoFile(stm, folder_key, std::move(callback)));
}

template <class TRequest>
GetInfoFile<TRequest>::GetInfoFile(SessionMaintainer * stm,
                                   const std::string & folder_key,
                                   CallbackType && callback)
        : stm_(stm), folder_key_(folder_key), callback_(callback)
{
}

template <class TRequest>
void GetInfoFile<TRequest>::Cancel()
{
    cancelled_ = true;

    if (request_ != nullptr)
        request_->Cancel();
}

template <class TRequest>
void GetInfoFile<TRequest>::CoroutineBody(pull_type & yield)
{
    auto self = shared_from_this();  // Hold a reference to our
    // object until the coroutine
    // is complete, otherwise
    // handler will have invalid
    // reference to this because
    // the base object has
    // disappeared from scope

    std::function<void(const ResponseType & response)> HandleFileGetInfo =
            [this, self](const ResponseType & response)
    {
        if (response.error_code)
        {
            // If there was an error, insert into vector and
            // propagate at the callback.
            std::string error_str = "No error string provided";
            if (response.error_string)
                error_str = *response.error_string;

            errors_.push_back(
                    ErrorType(folder_key_, response.error_code, error_str));
        }
        else
        {
            response_ = response;
        }

        // Resume the coroutine

        request_ = nullptr;  // Must free request_ or coroutine cannot be
        // destructed.
        Resume();
    };

    request_ = stm_->Call(RequestType(folder_key_), HandleFileGetInfo);

    if (cancelled_)
        request_->Cancel();

    yield();

    // Coroutine is done, so call the callback.
    callback_(response_, errors_);
}
}  // namespace mf
}  // namespace api