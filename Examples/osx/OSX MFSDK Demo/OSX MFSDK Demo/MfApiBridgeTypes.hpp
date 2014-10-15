/**
 * @file MfApiBridgeTypes.hpp
 * @author Zachary Marquez
 * @date 10/14/14
 * @copyright Copyright 2014 MediaFire, LLC. All rights reserved.
 */
#pragma once

#include <functional>
#include <string>
#include <vector>

namespace detail {

struct UserInfo
{
    std::string email;
    std::string firstName;
    std::string lastName;
};

enum class ContentType
{
    File,
    Folder,
};

struct ContentItem
{
    std::string name;
    ContentType type;
    std::string key;
    std::string date;
    std::string privacy;
};

typedef std::vector<ContentItem> FolderContents;

typedef std::function<void(int /*errorCode*/, const std::string& /*errorMsg*/)> FailureCallback;

typedef std::function<void()> LoginCallback;
typedef std::function<void(const UserInfo& /*userInfo*/)> GetUserInfoCallback;
typedef std::function<void(const FolderContents& /*folderContents*/, int /*chunk*/, bool /*moreChunks*/)> GetFolderContentCallback;

}  // namespace detail
