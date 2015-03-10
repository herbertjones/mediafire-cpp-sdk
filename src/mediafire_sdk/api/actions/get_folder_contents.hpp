#pragma once

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/folder/get_content.hpp"

#include "coroutine.hpp"

#include "boost/coroutine/all.hpp"

namespace mf
{
namespace api
{

class GetFolderContents : public Coroutine
{
public:
    using ContentType = mf::api::folder::get_content::ContentType;

    using ApiRequestType = mf::api::folder::get_content::Request;
    using ApiResponseType = mf::api::folder::get_content::Response;

    using File = ApiResponseType::File;
    using Folder = ApiResponseType::Folder;

    using CallbackType = std::function<void(std::vector<ApiResponseType::File>, std::vector<ApiResponseType::Folder>)>;

public:
    static std::shared_ptr<GetFolderContents> Create(SessionMaintainer * stm, const std::string & folder_key, const ContentType & content_type, CallbackType && callback)
    {
        return std::shared_ptr<GetFolderContents>(new GetFolderContents(stm, folder_key, content_type, std::move(callback)));
    }

    void operator()() override
    {
        coro_();
    }

    std::string GetFolderKey() { return folder_key_; }

private:
    GetFolderContents(SessionMaintainer * stm, const std::string & folder_key, const ContentType & content_type, CallbackType && callback) : stm_(stm), folder_key_(folder_key), content_type_(content_type), callback_(callback) {}

private:
    SessionMaintainer * stm_;

    std::string folder_key_;

    std::vector<File> files_;
    std::vector<Folder> folders_;

    ContentType content_type_;

    CallbackType callback_;

    push_type coro_
    {
        [this](pull_type & yield)
        {
            auto self = shared_from_this(); // Hold a reference to our object until the coroutine is complete, otherwise handler will have invalid reference to this

            int chunk_num = 1;
            bool chunks_remaining = true;
            while (chunks_remaining)
            {
                std::function<void(const ApiResponseType & response)> HandleFolderGetContents =
                    [this, self, &chunks_remaining](const ApiResponseType & response)
                    {
                        folders_.insert(std::end(folders_), std::begin(response.folders), std::end(response.folders));
                        files_.insert(std::end(files_), std::begin(response.files), std::end(response.files));

                        if (response.chunks_remaining == mf::api::folder::get_content::ChunksRemaining::LastChunk)
                            chunks_remaining = false;

                        (*this)();
                    };

                stm_->Call(ApiRequestType(folder_key_, chunk_num, content_type_),
                           HandleFolderGetContents);

                yield();

                ++chunk_num;
            }

            callback_(files_, folders_);
        }
    };
};

    } // namespace mf
} // namespace api