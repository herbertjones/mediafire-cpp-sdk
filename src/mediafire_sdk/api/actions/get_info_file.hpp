#pragma once

#include "mediafire_sdk/api/session_maintainer.hpp"

#include "coroutine.hpp"

#include "boost/coroutine/all.hpp"

namespace mf
{
namespace api
{

template <typename TRequest>
class GetInfoFile : public Coroutine
{
public:
    // Some convenience typedefs
    using RequestType = TRequest;
    using ResponseType = typename RequestType::ResponseType;

    // The struct for the errors we might return
    struct ErrorType
    {
        ErrorType() {}

        ErrorType(const std::string & folder_key,
                  const std::error_code & error_code,
                  const std::string & error_string);

        std::string folder_key;
        std::error_code error_code;
        std::string error_string;
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
    static std::shared_ptr<GetInfoFile> Create(SessionMaintainer * stm,
                                               const std::string & folder_key,
                                               CallbackType && callback);

    void Cancel() override;

private:
    /**
     *  @brief  Private constructor.
     **/
    GetInfoFile(SessionMaintainer * stm,
                const std::string & folder_key,
                CallbackType && callback);

private:
    SessionMaintainer * stm_;

    std::string folder_key_;

    CallbackType callback_;

    ResponseType response_;
    std::vector<ErrorType> errors_;

    bool cancelled_ = false;

    void CoroutineBody(pull_type & yield) override;
};

}  // namespace mf
}  // namespace api

#include "get_info_file_impl.hpp"