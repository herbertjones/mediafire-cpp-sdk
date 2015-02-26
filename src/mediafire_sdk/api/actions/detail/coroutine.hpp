/**
 * @file api/actions/detail/coroutine.hpp
 * @author Herbert Jones
 * @brief Implementaion for coroutine to be used as base for actions.
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include <memory>
#include <string>
#include <system_error>

#include "boost/asio/coroutine.hpp"
#include "boost/type_traits/function_traits.hpp"

#include "mediafire_sdk/api/session_maintainer.hpp"

#include "coroutine_base.hpp"

namespace mf
{
namespace api
{
namespace detail
{

/**
 * @class Coroutine
 * @brief Implementaion for coroutine to be used as base for actions.
 *
 * This class provides a set of convenience functions for an "action" class that
 * wants to encapsulate an often repeated or hard to get right procedure
 * involvind asynchronous operations, such as those required for efficiently
 * communicating with the cloud.
 *
 * This implements stackless co-routines.  When using resume/yield, do not leave
 * items on the stack and expect them to be there or initialized when resuming.
 */
template <typename T>
class Coroutine : public std::enable_shared_from_this<T>,
                  public boost::asio::coroutine,
                  public CoroutineBase
{
public:
    /** Shared pointer of the downcast class. */
    using Pointer = std::shared_ptr<T>;

    /** Callback signature to return the result of the operation. */
    using Callback = std::function<void(ActionResult, Pointer)>;

    /**
     * @brief Static method to create a Coroutine shared object.
     *
     * @param[in] stm Session manager
     * @param[in] callback This is called with the result of the operation.
     *                     Its signature is:
     *                     void(ActionResult, Pointer)
     * @param[in] args Arguments to pass to the constructor.
     *
     * The returned object will need to have Start() called on it.
     *
     * @return Shared pointer with new Coroutine.
     */
    template <typename... Args>
    static Pointer CreateUnstarted(mf::api::SessionMaintainer * stm,
                                   Callback callback,
                                   Args &&... args)
    {
        // Give less confusting error message if constructor doesn't exist.
        // Only works if friend added to class.
        // using FailToCompileIfNoMatchingConstructor
        //         = decltype(T(stm, callback, std::forward<Args>(args)...));

        struct ConstructPermitter : public T
        {
            ConstructPermitter(mf::api::SessionMaintainer * stm,
                               Callback callback,
                               Args &&... args)
                    : T(stm, callback, std::forward<Args>(args)...)
            {
            }
        };

        auto ptr = std::make_shared<ConstructPermitter>(
                stm, callback, std::forward<Args>(args)...);

        return ptr;
    }

    /**
     * @brief Static method to create a Coroutine shared object.
     *
     * @param[in] stm Session manager
     * @param[in] callback This is called with the result of the operation.
     *                     Its signature is:
     *                     void(ActionResult, Pointer)
     * @param[in] args Arguments to pass to the constructor.
     *
     * Object to continue to exist until complete and callback called..
     *
     * @return Shared pointer with new Coroutine.
     */
    template <typename... Args>
    static Pointer Create(mf::api::SessionMaintainer * stm,
                          Callback callback,
                          Args &&... args)
    {
        auto ptr = CreateUnstarted(stm, callback, std::forward<Args>(args)...);

        // Start
        ptr->Start();

        return ptr;
    }

    virtual ~Coroutine() {}

    /**
     * @brief Start a coroutine that was created via CreateUnstarted.
     */
    void Start()
    {
        // From start to finish, object life is maintained.
        delete_preventer_ = this->shared_from_this();
        (*static_cast<T *>(this))();
    }

protected:
    /**
     * @brief Makes an API request.  Must yield when called.
     *
     * Boilerplate to easily make an API request.  Doesn't return anything as
     * this co-routine is stackless.
     *
     * @param[in] request The API request to pass to SessionMaintainer::Call.
     * @param[out] response The API response location where to store the
     *                      response of Call().
     */
    template <typename RequestType, typename ResponseType>
    void CallApi(RequestType request, ResponseType & store)
    {
        auto self = this->shared_from_this();
        stm_->Call(std::move(request),
                   [this, self, &store](ResponseType response)
                   {
                       store = std::move(response);

                       // Resume coroutine.
                       (*self)();
                   });
    }

    /**
     * @brief Calls another action.  Must yield when called.
     *
     * Boilerplate to easily call another action.  Doesn't return anything as
     * this co-routine is stackless.
     *
     * @param[out] request Action pointer to store the created action.
     * @param[in] args Arguments to pass to the Create method of the given
     *                 action.
     */
    template <typename ActionPtr, typename... Args>
    void CallAction(ActionPtr & action_ptr, Args &&... args)
    {
        using Coro = typename ActionPtr::element_type;

        auto self = this->shared_from_this();

        action_ptr = Coro::Create(stm_,
                                  [self, this](ActionResult, ActionPtr)
                                  {
                                      // Resume coroutine.
                                      (*self)();
                                  },
                                  std::forward<Args>(args)...);
    }

    Coroutine(mf::api::SessionMaintainer * stm, Callback callback)
            : stm_(stm), callback_(callback)
    {
    }

    /**
     * @brief End processing with success.
     *
     * This must be called once the action has successfully completed.  The
     * memory allocated can only be freed once this or ReturnFailure is called.
     */
    void ReturnSuccess()
    {
        action_result_ = ActionResult::Success;

        auto self = this->shared_from_this();

        // This object is now allowed to be destroyed.
        delete_preventer_.reset();

        stm_->HttpConfig()->GetDefaultCallbackIoService()->post(
                [self, this]()
                {
                    callback_(ActionResult::Success, self);
                });
    }

    /**
     * @brief End processing with Failure.
     *
     * This must be called once the action has successfully completed.  The
     * memory allocated can only be freed once this or ReturnFailure is called.
     *
     * @param[in] ec Error code
     * @param[in] description Text descript of error if available.
     */
    void ReturnFailure(std::error_code ec,
                       boost::optional<std::string> description)
    {
        action_result_ = ActionResult::Failure;
        error_code_ = ec;
        error_description_ = description;

        auto self = this->shared_from_this();

        // This object is now allowed to be destroyed.
        delete_preventer_.reset();

        stm_->HttpConfig()->GetDefaultCallbackIoService()->post(
                [self, this]()
                {
                    callback_(ActionResult::Failure, self);
                });
    }

    /**
     * @brief End processing with Failure.
     *
     * This must be called once the action has successfully completed.  The
     * memory allocated can only be freed once this or ReturnFailure is called.
     *
     * @param[in] action_ptr Sub-action in error state.
     */
    template <typename ActionPtr>
    void ReturnFailure(ActionPtr & action_ptr)
    {
        ReturnFailure(action_ptr->GetErrorCode(),
                      action_ptr->GetErrorDescription());
    }

protected:
    mf::api::SessionMaintainer * stm_;
    Callback callback_;
    Pointer delete_preventer_;

private:
};

}  // namespace detail
}  // namespace api
}  // namespace mf
