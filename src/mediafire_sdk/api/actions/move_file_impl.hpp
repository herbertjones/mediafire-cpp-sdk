namespace mf
{
namespace api
{

template <class TRequest>
MoveFile<TRequest>::ErrorType::ErrorType(
        const std::error_code & error_code,
        boost::optional<std::string> error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<MoveFile<TRequest>> MoveFile<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & quick_key,
        const std::string & destination_parent_folder_key,
        CallbackType && callback)
{
    return std::shared_ptr<MoveFile>(new MoveFile(stm,
                                                  quick_key,
                                                  destination_parent_folder_key,
                                                  std::move(callback)));
}

template <class TRequest>
MoveFile<TRequest>::MoveFile(SessionMaintainer * stm,
                             const std::string & quick_key,
                             const std::string & destination_parent_folder_key,
                             CallbackType && callback)
        : stm_(stm),
          quick_key_(quick_key),
          destination_parent_folder_key_(destination_parent_folder_key),
          callback_(callback)
{
}

template <class TRequest>
void MoveFile<TRequest>::Cancel()
{

    cancelled_ = true;

    if (request_ != nullptr)
        request_->Cancel();
}

template <class TRequest>
void MoveFile<TRequest>::CoroutineBody(pull_type & yield)
{
    auto self = shared_from_this();  // Hold a reference to our
    // object until the coroutine
    // is complete, otherwise
    // handler will have invalid
    // reference to this because
    // the base object has
    // disappeared from scope

    std::function<void(const ResponseType & response)> HandleMoveFile =
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

        // Resume the coroutine
        Resume();
    };

    request_ = stm_->Call(
            RequestType(quick_key_, destination_parent_folder_key_),
            HandleMoveFile);

    if (cancelled_)
        request_->Cancel();

    yield();

    // Coroutine is done, so call the callback.
    callback_(response_, errors_);
}

}  // namespace mf
}