namespace mf
{
namespace api
{

template <class TRequest>
GetInfoFolder<TRequest>::ErrorType::ErrorType(
        const std::string & folder_key,
        const std::error_code & error_code,
        const boost::optional<std::string> & error_string)
        : folder_key(folder_key),
          error_code(error_code),
          error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<GetInfoFolder<TRequest>> GetInfoFolder<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & folder_key,
        CallbackType && callback)
{
    return std::shared_ptr<GetInfoFolder>(
            new GetInfoFolder(stm, folder_key, std::move(callback)));
}

template <class TRequest>
GetInfoFolder<TRequest>::GetInfoFolder(SessionMaintainer * stm,
                                       const std::string & folder_key,
                                       CallbackType && callback)
        : stm_(stm), folder_key_(folder_key), callback_(callback)
{
}

template <class TRequest>
void GetInfoFolder<TRequest>::Cancel()
{
    cancelled_ = true;

    if (request_ != nullptr)
        request_->Cancel();
}

template <class TRequest>
void GetInfoFolder<TRequest>::CoroutineBody(pull_type & yield)
{
    auto self = shared_from_this();  // Hold a reference to our
    // object until the coroutine
    // is complete, otherwise
    // handler will have invalid
    // reference to this because
    // the base object has
    // disappeared from scope

    std::function<void(const ResponseType & response)> HandleFolderGetInfo =
            [this, self](const ResponseType & response)
    {
        if (response.error_code)
        {
            errors_.push_back(ErrorType(
                    folder_key_, response.error_code, response.error_string));
        }
        else
        {
            response_ = response;
        }

        request_ = nullptr;  // Must free request_ or coroutine cannot be
        // destructed.

        // Resume the coroutine
        Resume();
    };

    request_ = stm_->Call(RequestType(folder_key_), HandleFolderGetInfo);

    if (cancelled_)
        request_->Cancel();

    yield();

    // Coroutine is done, so call the callback.
    callback_(response_, errors_);
}
}  // namespace mf
}  // namespace api