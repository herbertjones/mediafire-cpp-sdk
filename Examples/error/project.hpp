/**
 * @file project.hpp
 * @author Herbert Jones
 * @brief Project error codes.
 *
 * Copyright 2014 Mediafire
 *
 * Each module may have its own error codes. However, we need a way to ensure
 * that error codes are unique across the project. By maintaining the range here
 * for each module, then the module can be ported to another application easily.
 *
 * Each module here should use its own namespace. The module should start where
 * the last one left off. The modules should be ordered correctly.
 *
 * error::MODULE::start is where error codes for that module should start.
 * error::MODULE::end is where error codes for that module end, non inclusive.
 */
#pragma once

namespace error
{
    namespace http
    {
        enum { start = 10000, end = 11000 };
    }  // End namespace http

    namespace api
    {
        enum { start = 11000, end = 12000 };
    }  // End namespace api
}  // End namespace error
