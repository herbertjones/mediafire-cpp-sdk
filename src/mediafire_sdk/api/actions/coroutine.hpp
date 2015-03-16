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
    virtual void operator()() = 0;
};

}  // namespace api
}  // namespace mf