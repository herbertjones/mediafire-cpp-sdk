#pragma once

#include "mediafire_sdk/api/session_maintainer.hpp"

#include "coroutine.hpp"

#include "boost/coroutine/all.hpp"

namespace mf
{
namespace api
{

template <typename TRequest>
class DeleteFile : public Coroutine
{
public:
    // Some convenience typedefs
    using RequestType = TRequest;
    using ResponseType = typename RequestType::ResponseType;

    // The struct for the errors we might return
    struct ErrorType
    {
        ErrorType() {}

        ErrorType(const std::error_code & error_code,
                  boost::optional<std::string> error_string);

        std::error_code error_code;
        boost::optional<std::string> error_string;
    };

    using CallbackType
            = std::function<void(const ResponseType & response,
                                 const std::vector<ErrorType> & errors)>;

public:
    /**
     *  @brief Create an instance and get us the shared pointer to the created
     *instance.
     *
     *  @return std::shared_ptr Shared pointer to the created instance.
     **/
    static std::shared_ptr<DeleteFile> Create(SessionMaintainer * stm,
                                              const std::string & quick_key,
                                              CallbackType && callback);

    void Cancel() override;

private:
    /**
     *  @brief  Private constructor.
     **/
    DeleteFile(SessionMaintainer * stm,
               const std::string & quick_key,
               CallbackType && callback);

private:
    SessionMaintainer * stm_;

    std::string quick_key_;

    CallbackType callback_;

    ResponseType response_;
    std::vector<ErrorType> errors_;

    void CoroutineBody(pull_type & yield) override;
};

}  // namespace mf
}  // namespace api

#include "delete_file_impl.hpp"