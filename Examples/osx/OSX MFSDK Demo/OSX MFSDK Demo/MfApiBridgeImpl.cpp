/**
 * @file MfApiBridgeImpl.cpp
 * @author Zachary Marquez
 * @date 10/13/14
 * @copyright Copyright 2014 MediaFire, LLC. All rights reserved.
 */
#include "MfApiBridgeImpl.hpp"

#include "MfApiBridgeHelper.hpp"

namespace detail {

MfApiBridgeImpl::MfApiBridgeImpl() :
    impl_(new MfApiBridgeHelper(this))
{
}

MfApiBridgeImpl::~MfApiBridgeImpl()
{
    impl_.reset();
}

void MfApiBridgeImpl::LogIn(
        std::string email,
        std::string password,
        LoginCallback onSuccess,
        FailureCallback onFailure
    )
{
    impl_->LogIn(email, password, onSuccess, onFailure);
}

void MfApiBridgeImpl::GetUserInfo(
        GetUserInfoCallback onSuccess,
        FailureCallback onFailure
    )
{
    impl_->GetUserInfo(onSuccess, onFailure);
}

void MfApiBridgeImpl::GetFolderContent(
        const std::string& folderkey,
        ContentType contentType,
        int chunk,
        GetFolderContentCallback onSuccess,
        FailureCallback onFailure
    )
{
    impl_->GetFolderContent(
            folderkey,
            contentType,
            chunk,
            onSuccess,
            onFailure
        );
}

}  // namespace detail
