namespace mf
{
namespace api
{

template <class TRequest>
DeleteFolder<TRequest>::ErrorType::ErrorType(
        const std::error_code & error_code,
        boost::optional<std::string> error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<DeleteFolder<TRequest>> DeleteFolder<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & folder_key,
        CallbackType && callback)
{
    return std::shared_ptr<DeleteFolder>(
            new DeleteFolder(stm, folder_key, std::move(callback)));
}

template <class TRequest>
DeleteFolder<TRequest>::DeleteFolder(SessionMaintainer * stm,
                                     const std::string & folder_key,
                                     CallbackType && callback)
        : stm_(stm), folder_key_(folder_key), callback_(callback)
{
}

template <class TRequest>
void DeleteFolder<TRequest>::Cancel()
{
}

template <class TRequest>
void DeleteFolder<TRequest>::CoroutineBody(pull_type & yield)
{
    auto self = shared_from_this();  // Hold a reference to our
    // object until the coroutine
    // is complete, otherwise
    // handler will have invalid
    // reference to this because
    // the base object has
    // disappeared from scope

    std::function<void(const ResponseType & response)> HandleDeleteFolder =
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

    stm_->Call(RequestType(folder_key_), HandleDeleteFolder);

    yield();

    // Coroutine is done, so call the callback.
    callback_(response_, errors_);
}
}  // namespace mf
}  // namespace api