/**
 * @file MfApiBridgeImpl.hpp
 * @author Zachary Marquez
 * @date 10/13/14
 * @copyright Copyright 2014 MediaFire, LLC. All rights reserved.
 */
#pragma once

#include <functional>
#include <memory>
#include <string>

#include "MfApiBridgeTypes.hpp"

namespace detail {

class MfApiBridgeHelper;

/*
 * This class simply forwards calls from Objective-C++ code to pure C++ code.
 * It is necessary because the boost headers do not compile under the
 * Objective-C++ compiler.
 */
class MfApiBridgeImpl
{
public:
    MfApiBridgeImpl();
    ~MfApiBridgeImpl();

    void LogIn(
            std::string email,
            std::string password,
            LoginCallback onSuccess,
            FailureCallback onFailure
        );

    void GetUserInfo(
            GetUserInfoCallback onSuccess,
            FailureCallback onFailure
        );

    void GetFolderContent(
            const std::string& folderkey,
            ContentType contentType,
            int chunk,
            GetFolderContentCallback onSuccess,
            FailureCallback onFailure
        );

private:
    std::unique_ptr<MfApiBridgeHelper> impl_;
};

}  // namespace detail
