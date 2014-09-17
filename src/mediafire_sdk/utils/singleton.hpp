/*
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include "boost/thread/locks.hpp"
#include "boost/thread/mutex.hpp"

namespace mf {
namespace utils
{
    /**
     * @brief This class allows you to create a singleton class.
     *
     * This is a class that:
     *   1) Can only have one instance in existance at any given time
     *   2) Is allocated only when it is requested.
     * Therefore, only one instance will exist and does not exist until it is 
     * created).
     *
     * @tparam ObjectT The type of the class you want to make a singleton.
     *
     * @note See "logger/logger.hpp" and logger/unit_tests/testLogging.cpp for
     *       an example of how to use this class. Note the class access
     *       specifiers, inheritance, and the friending.
     **/
template < class ObjectT >
class Singleton
{
public:
    /**
     * Return a reference to the ObjectT instance or creates one if it
     * doesn't exist.
     *
     * @return A reference to the ObjectT instance.
     */
    static ObjectT & instance()
    {
        // Ensure thread-safety
        boost::lock_guard< boost::mutex > lock(instance_mutex_);

        static ObjectT instance;

        return instance;
    }

protected:
    /// Prevent direct instantiation of singleton class
    Singleton() {}

    /// Prevent direct destruction of singleton class
    ~Singleton() {}

private:
    /// Purposefully unimplemented to prevent copying.
    Singleton(const Singleton &);

    /// Purposefully unimplemented to prevent moving.
    Singleton& operator= (const Singleton &);

    /// Mutex for thread safety for getting/making the instance
    static boost::mutex instance_mutex_;
};

/// Static instantation for mutex object
template < class ObjectT >
boost::mutex Singleton< ObjectT >::instance_mutex_;

}  // namespace utils
}  // namespace mf
