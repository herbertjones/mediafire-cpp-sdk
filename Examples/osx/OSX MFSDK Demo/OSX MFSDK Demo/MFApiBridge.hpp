/**
 * @file MFApiBridge.hpp
 * @author Zachary Marquez
 * @date 10/13/14
 * @copyright Copyright 2014 MediaFire, LLC. All rights reserved.
 */
#pragma once

#include <cocoa/cocoa.h>

#include <functional>
#include <memory>
#include <string>

#include "MfApiBridgeTypes.hpp"

extern NSString * const skDemoUserEmailKey;
extern NSString * const skDemoUserFirstNameKey;
extern NSString * const skDemoUserLastNameKey;

extern NSString * const skDemoFolderContentNameKey;
extern NSString * const skDemoFolderContentTypeKey;
extern NSString * const skDemoFolderContentKeyKey;
extern NSString * const skDemoFolderContentDateKey;
extern NSString * const skDemoFolderContentPrivacyKey;

namespace detail { class MfApiBridgeImpl; }

/*
 * This class abstracts the MFAPI layer from Objective-C++ code and essentiallly
 * functions to translate between Objective-C and C++ data types and receive
 * callbacks from the api thread and put them onto the main thread.
 */
class MfApiBridge :
    public std::enable_shared_from_this<MfApiBridge>
{
public:
    enum class ContentType
    {
        Files,
        Folders,
    };

    typedef std::shared_ptr<MfApiBridge> Pointer;

    typedef void (^LoginBlock)(Pointer /*apiBridge*/);
    typedef void (^GetUserInfoBlock)(NSDictionary* /*userInfo*/);
    typedef void (^GetFolderContentBlock)(NSArray* /*folderContent*/, NSInteger /*chunk*/, BOOL /*moreChunks*/);

    typedef void (^FailureBlock)(int /*errorCode*/, NSString* /*errorMsg*/);

    static void LogIn(
            NSString* email,
            NSString* password,
            LoginBlock onSuccess,
            FailureBlock onFailure
        );

    ~MfApiBridge();

    void GetUserInfo(
            GetUserInfoBlock onSuccess,
            FailureBlock onFailure
        );

    void GetFolderContent(
            NSString* folderkey,
            ContentType contentType,
            NSInteger chunk,
            GetFolderContentBlock onSuccess,
            FailureBlock onFailure
        );

private:
    MfApiBridge();
    MfApiBridge(const MfApiBridge&) = delete;

    void Init();

    void LoginCallback(LoginBlock loginBlock);
    void UserInfoCallback(GetUserInfoBlock userInfoBlock, const detail::UserInfo& userInfo);
    void GetFolderContentCallback(GetFolderContentBlock getFolderContentBlock, const detail::FolderContents& folderContents, int chunk, bool moreChunks);
    void FailureCallback(FailureBlock failureBlock, int errorCode, const std::string& errorMessage);

    std::unique_ptr<detail::MfApiBridgeImpl> impl_;
};
