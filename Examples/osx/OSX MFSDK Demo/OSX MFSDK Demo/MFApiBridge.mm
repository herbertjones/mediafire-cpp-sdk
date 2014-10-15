/**
 * @file MFApiBridge.mm
 * @author Zachary Marquez
 * @date 10/13/14
 * @copyright Copyright 2014 MediaFire, LLC. All rights reserved.
 */
#include "MFApiBridge.hpp"

#include "MfApiBridgeImpl.hpp"

NSString * const skDemoUserEmailKey = @"DemoUserEmailKey";
NSString * const skDemoUserFirstNameKey = @"DemoUserFirstNameKey";
NSString * const skDemoUserLastNameKey = @"DemoUserLastNameKey";

NSString * const skDemoFolderContentNameKey = @"DemoFolderContentNameKey";
NSString * const skDemoFolderContentTypeKey = @"DemoFolderContentTypeKey";
NSString * const skDemoFolderContentKeyKey = @"DemoFolderContentKeyKey";
NSString * const skDemoFolderContentDateKey = @"DemoFolderContentDateKey";
NSString * const skDemoFolderContentPrivacyKey = @"DemoFolderContentPrivacyKey";


void MfApiBridge::LogIn(
        NSString* email,
        NSString* password,
        LoginBlock onSuccess,
        FailureBlock onFailure
    )
{
    using namespace std::placeholders;
    
    Pointer newBridge(new MfApiBridge);
    newBridge->Init();
    newBridge->impl_->LogIn(
            [email UTF8String],
            [password UTF8String],
            std::bind(
                &MfApiBridge::LoginCallback,
                newBridge,
                onSuccess
            ),
            std::bind(
                &MfApiBridge::FailureCallback,
                newBridge,
                onFailure,
                _1,
                _2
            )
        );
}

MfApiBridge::MfApiBridge() :
    impl_()
{
}

MfApiBridge::~MfApiBridge()
{
}

void MfApiBridge::Init()
{
    impl_.reset(new detail::MfApiBridgeImpl());
}

void MfApiBridge::GetUserInfo(
        GetUserInfoBlock onSuccess,
        FailureBlock onFailure
    )
{
    using namespace std::placeholders;
    impl_->GetUserInfo(
            std::bind(
                &MfApiBridge::UserInfoCallback,
                shared_from_this(),
                onSuccess,
                _1
            ),
            std::bind(
                &MfApiBridge::FailureCallback,
                shared_from_this(),
                onFailure,
                _1,
                _2
            )
        );
}

void MfApiBridge::GetFolderContent(
        NSString* folderkey,
        ContentType contentType,
        NSInteger chunk,
        GetFolderContentBlock onSuccess,
        FailureBlock onFailure
    )
{
    using namespace std::placeholders;
    detail::ContentType type = ( contentType == ContentType::Files ?
        detail::ContentType::File : detail::ContentType::Folder );
    impl_->GetFolderContent(
            [folderkey UTF8String],
            type,
            static_cast<int>(chunk),
            std::bind(
                &MfApiBridge::GetFolderContentCallback,
                shared_from_this(),
                onSuccess,
                _1,
                _2,
                _3
            ),
            std::bind(
                &MfApiBridge::FailureCallback,
                shared_from_this(),
                onFailure,
                _1,
                _2
            )
        );
}

void MfApiBridge::LoginCallback(LoginBlock loginBlock)
{
    MfApiBridge::Pointer myself = shared_from_this();
    dispatch_async(dispatch_get_main_queue(), ^{
            loginBlock(myself);
        });
}

void MfApiBridge::UserInfoCallback(
        GetUserInfoBlock userInfoBlock,
        const detail::UserInfo& userInfo
    )
{
    NSDictionary* userDict = @{
        skDemoUserEmailKey: [NSString stringWithUTF8String:userInfo.email.c_str()],
        skDemoUserFirstNameKey: [NSString stringWithUTF8String:userInfo.firstName.c_str()],
        skDemoUserLastNameKey: [NSString stringWithUTF8String:userInfo.lastName.c_str()]
        };
    dispatch_async(dispatch_get_main_queue(), ^{
            userInfoBlock(userDict);
        });
}

void MfApiBridge::GetFolderContentCallback(
        GetFolderContentBlock getFolderContentBlock,
        const detail::FolderContents& folderContents,
        int chunk,
        bool moreChunks
    )
{
    NSMutableArray* nodes = [NSMutableArray array];
    for (const detail::ContentItem& item : folderContents)
    {
        NSDictionary* node = @{
            skDemoFolderContentNameKey: [NSString stringWithUTF8String:item.name.c_str()],
            skDemoFolderContentTypeKey: (item.type == detail::ContentType::File ? @"File" : @"Folder"),
            skDemoFolderContentKeyKey: [NSString stringWithUTF8String:item.key.c_str()],
            skDemoFolderContentDateKey: [NSString stringWithUTF8String:item.date.c_str()],
            skDemoFolderContentPrivacyKey: [NSString stringWithUTF8String:item.privacy.c_str()]
            };
        [nodes addObject:node];
    }
    dispatch_async(dispatch_get_main_queue(), ^{
            getFolderContentBlock(
                nodes,
                static_cast<NSInteger>(chunk),
                static_cast<BOOL>(moreChunks)
            );
        });
}

void MfApiBridge::FailureCallback(FailureBlock failureBlock, int errorCode, const std::string& errorMessage)
{
    // It is necessary that we pass a pointer to the block that will be executed
    // in the main queue so that destruction happens there. If destruction were
    // to happen on the API thread then the thread join would not be possible
    // and we would likely crash.
    __block MfApiBridge::Pointer keepAlive = shared_from_this();
    NSString* nsErrorMessage = [NSString stringWithUTF8String:errorMessage.c_str()];
    dispatch_async(dispatch_get_main_queue(), ^{
            failureBlock(errorCode, nsErrorMessage);
            keepAlive.reset();
        });
}
