#pragma once

#include <memory>

#include "boost/coroutine/all.hpp"

namespace mf
{
namespace api
{

class Coroutine : public boost::coroutines::coroutine<void>,
                  public std::enable_shared_from_this<Coroutine>
{
public:
    push_type coro_{[this](pull_type & yield)
                    {
                        CoroutineBody(yield);
                    }};

    virtual void Start() { coro_(); }

    virtual void Resume() { coro_(); }

    virtual void Cancel() = 0;

private:
    virtual void CoroutineBody(pull_type & yield) = 0;
};

}  // namespace api
}  // namespace mf