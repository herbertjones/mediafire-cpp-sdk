namespace mf
{
namespace api
{

template <class TRequest>
CreateFolder<TRequest>::ErrorType::ErrorType(
        const std::error_code & error_code,
        boost::optional<std::string> error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<CreateFolder<TRequest>> CreateFolder<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & folder_name,
        const std::string & parent_folder_key,
        CallbackType && callback)
{
    return std::shared_ptr<CreateFolder>(new CreateFolder(
            stm, folder_name, parent_folder_key, std::move(callback)));
}

template <class TRequest>
CreateFolder<TRequest>::CreateFolder(SessionMaintainer * stm,
                                     const std::string & folder_name,
                                     const std::string & parent_folder_key,
                                     CallbackType && callback)
        : stm_(stm),
          folder_name_(folder_name),
          parent_folder_key_(parent_folder_key),
          callback_(callback)
{
}

template <class TRequest>
void CreateFolder<TRequest>::Cancel()
{
}

template <class TRequest>
void CreateFolder<TRequest>::CoroutineBody(pull_type & yield)
{
    auto self = shared_from_this();  // Hold a reference to our
    // object until the coroutine
    // is complete, otherwise
    // handler will have invalid
    // reference to this because
    // the base object has
    // disappeared from scope

    std::function<void(const ResponseType & response)> HandleCreateFolder =
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

    auto request = RequestType(folder_name_);
    request.SetParentFolderkey(parent_folder_key_);

    stm_->Call(request, HandleCreateFolder);

    yield();

    // Coroutine is done, so call the callback.
    callback_(response_, errors_);
}
}  // namespace mf
}  // namespace api