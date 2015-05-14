namespace mf
{
namespace api
{

template <class TRequest>
DeleteFile<TRequest>::ErrorType::ErrorType(
        const std::error_code & error_code,
        boost::optional<std::string> error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<DeleteFile<TRequest>> DeleteFile<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & quick_key,
        CallbackType && callback)
{
    return std::shared_ptr<DeleteFile>(
            new DeleteFile(stm, quick_key, std::move(callback)));
}

template <class TRequest>
DeleteFile<TRequest>::DeleteFile(SessionMaintainer * stm,
                                 const std::string & quick_key,
                                 CallbackType && callback)
        : stm_(stm), quick_key_(quick_key), callback_(callback)
{
}

template <class TRequest>
void DeleteFile<TRequest>::Cancel()
{
    cancelled_ = true;
    if (request_ != nullptr)
        request_->Cancel();
}

template <class TRequest>
void DeleteFile<TRequest>::CoroutineBody(pull_type & yield)
{
    auto self = shared_from_this();  // Hold a reference to our
    // object until the coroutine
    // is complete, otherwise
    // handler will have invalid
    // reference to this because
    // the base object has
    // disappeared from scope

    std::function<void(const ResponseType & response)> HandleDeleteFile =
            [this, self](const ResponseType & response)
    {
        if (response.error_code)
        {
            // If there was an error, insert into vector and
            // propagate at the callback.

            errors_.push_back(
                    ErrorType(response.error_code, response.error_string));
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

    request_ = stm_->Call(RequestType(quick_key_), HandleDeleteFile);

    if (cancelled_)
        request_->Cancel();

    yield();

    // Coroutine is done, so call the callback.
    callback_(response_, errors_);
}

}  // namespace mf
}  // namespace api