/**
 * @file transition_upload.hpp
 * @author Herbert Jones
 * @brief State machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <iostream>
#include <map>

#include "mediafire_sdk/http/http_request.hpp"
#include "mediafire_sdk/http/post_data_pipe_interface.hpp"

#include "boost/algorithm/string/join.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/variant/apply_visitor.hpp"
#include "boost/property_tree/json_parser.hpp"

#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/api/response_base.hpp"

#include "mediafire_sdk/uploader/detail/hasher_events.hpp"
#include "mediafire_sdk/uploader/detail/types.hpp"
#include "mediafire_sdk/uploader/detail/upload_events.hpp"
#include "mediafire_sdk/uploader/detail/upload_target.hpp"
#include "mediafire_sdk/uploader/error.hpp"
#include "mediafire_sdk/uploader/upload_request.hpp"

#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/utils/url_encode.hpp"

namespace mf {
namespace uploader {
namespace detail {
namespace upload_transition {

std::string AsDateTime(std::time_t datetime);

std::string AssembleQuery(
        const std::map< std::string, std::string > & query_map
    );

class TargetUrlVisitor : public boost::static_visitor<>
{
public:
    TargetUrlVisitor(
        std::map< std::string, std::string > * query_map
    ) :
        query_map_(*query_map) {}

    void operator()(mf::uploader::detail::ParentFolderKey & variant) const
    {
        if ( ! variant.key.empty() )
            query_map_["folder_key"] = variant.key;
    }

    void operator()(mf::uploader::detail::CloudPath & variant) const
    {
        query_map_["path"] = mf::utils::path_to_utf8(variant.path);
    }

private:
    std::map< std::string, std::string > & query_map_;
};

class UploadPostDataPipe :
    public mf::http::PostDataPipeInterface
{
public:
    UploadPostDataPipe(
            mf::utils::FileIO::Pointer file,
            const uint64_t start_byte,
            const uint64_t end_byte
        ) :
        file_(file),
        start_byte_(start_byte),
        end_byte_(end_byte),
        current_read_pos_(start_byte_)
    {
        assert(file_);

        // Range should be correct and never empty
        assert(PostDataSize() > 0);
        assert(start_byte < end_byte);
    }

    virtual uint64_t PostDataSize() const
    {
        return end_byte_ - start_byte_;
    }

    virtual mf::http::SharedBuffer::Pointer RetreivePostDataChunk()
    {
        assert( file_->Tell() == current_read_pos_ );

        if (file_->Tell() != current_read_pos_)
        {
            std::error_code ec;
            file_->Seek(mf::utils::SeekAnchor::Beginning, current_read_pos_,
                &ec);

            if (ec)
            {
                /** @todo hjones: Emit error */
                return mf::http::SharedBuffer::Pointer();
            }
        }

        const uint64_t max_buffer_size = 1024 * 8;
        const uint64_t remaining_bytes = end_byte_ - current_read_pos_;

        if (remaining_bytes == 0)
        {
            // Done
            return mf::http::SharedBuffer::Pointer();
        }

        const uint64_t buffer_size = std::min(max_buffer_size, remaining_bytes);

        auto buffer = mf::http::SharedBuffer::Create(buffer_size);
        assert(buffer->Size() == buffer_size);

        std::error_code ec;
        const auto bytes_read = file_->Read(buffer->Data(), buffer_size, &ec);

        if (ec)
        {
            /** @todo hjones: Emit error */
            return mf::http::SharedBuffer::Pointer();
        }

        if (bytes_read != buffer_size)
        {
            /** @todo hjones: Emit error */
            return mf::http::SharedBuffer::Pointer();
        }

        current_read_pos_ += bytes_read;

        assert(current_read_pos_ <= end_byte_);
        return buffer;
    }

private:
    mf::utils::FileIO::Pointer file_;

    const uint64_t start_byte_;
    const uint64_t end_byte_;
    uint64_t current_read_pos_;

    mf::http::SharedBuffer::Pointer buffer_;
};

template <typename FSM>
void HandleUploadResponse(
    FSM & fsm,
    const std::string & url,
    const mf::http::Headers & headers,
    const std::string & content
)
{
    mf::api::ResponseBase response;
    response.InitializeWithContent(
        url,
        "",
        headers,
        content
        );

    if (response.error_code)
    {
        // Upload has unique negative values.
        fsm.ProcessEvent(event::Error{
            response.error_code,
            (response.error_string?*response.error_string:"Upload rejected")
            });
    }
    else
    {
        // We need the upload key to proceed.
        std::string upload_key;
        if ( mf::api::GetIfExists( response.pt, "response.doupload.key",
                &upload_key ) )
        {
            fsm.ProcessEvent( event::SimpleUploadComplete{upload_key});
        }
        else
        {
            int error = 0;
            if ( mf::api::GetIfExists( response.pt, "response.doupload.result",
                    &error ) )
            {
                fsm.ProcessEvent(event::Error{
                    std::error_code( error, upload_response_category() ),
                    "Upload rejected"
                    });
            }
            else
            {
                fsm.ProcessEvent(event::Error{
                    make_error_code(mf::api::api_code::ContentInvalidData),
                    "Upload response missing uploadkey"
                    });
            }
        }
    }
}

struct DoSimpleUpload
{
    template <typename Event, typename FSM, typename SourceState,
             typename TargetState>
    void operator()(
            Event const &,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        Upload(fsm);
    }

    template <typename FSM>
    std::string BuildUrl(
            FSM & fsm
        )
    {
        std::string url = "http://www.mediafire.com/api/upload/simple.php";

        std::map< std::string, std::string > query_map;

        // Action token
        query_map["session_token"] = fsm.ActionToken();

        // Use JSON
        query_map["response_format"] = "json";

        if (fsm.OnDuplicateAction() == OnDuplicateAction::Replace)
            query_map["action_on_duplicate"] = "replace";

        // Use JSON
        query_map["mtime"] = AsDateTime(fsm.Mtime());

        UploadTarget target_folder = fsm.TargetFolder();
        boost::apply_visitor(TargetUrlVisitor(&query_map), target_folder);

        /** @todo hjones: Query map overrides */

        return url + AssembleQuery(query_map);
    }

    template <typename FSM>
    void SetHeaders(
            mf::http::HttpRequest * http_request,
            FSM & fsm
        )
    {
        assert(http_request);

        http_request->SetHeader("Content-Type", "application/octet-stream");

        // This is not required according to the documentation, but the server
        // is rejecting the upload without it.
        http_request->SetHeader("x-filename", fsm.Filename());

        { /* x-filesize */
            std::ostringstream ss;
            ss << fsm.Filesize();
            http_request->SetHeader("x-filesize", ss.str());
        }
    }

    template <typename FSM>
    void Upload(
            FSM & fsm
        )
    {
        auto ec = std::error_code();
        auto file_io = mf::utils::FileIO::Open(fsm.Path(), "r", &ec);
        if (ec)
        {
            fsm.ProcessEvent(event::Error{ec, "Unable to open file."});
            return;
        }

        file_io->Seek(mf::utils::SeekAnchor::Beginning, 0, &ec);
        assert(0 == file_io->Tell());

        if (ec)
        {
            fsm.ProcessEvent(event::Error{ec, "Seek file failed."});
            return;
        }

        // Add data to send as POST.
        auto pipe = std::make_shared<UploadPostDataPipe>(file_io, 0,
            fsm.Filesize());
        auto url = BuildUrl(fsm);

        auto fsmp = fsm.AsFrontShared();
        auto request = mf::http::HttpRequest::Create(
            fsm.GetSessionMaintainer()->HttpConfig(),
            [fsmp, url](mf::http::HttpRequest::CallbackResponse response)
            {
                if (response.error_code)
                {
                    if (response.error_text)
                    {
                        fsmp->ProcessEvent(event::Error{
                            response.error_code,
                            *response.error_text
                            });
                    }
                    else
                    {
                        fsmp->ProcessEvent(event::Error{
                            response.error_code,
                            "Unknown error"
                            });
                    }
                }
                else
                {
                    HandleUploadResponse(*fsmp.get(), url, response.headers,
                        response.content);
                }
            },
            url);

        SetHeaders(request.get(), fsm);

        request->SetPostDataPipe(pipe);

        request->Start();

        fsm.SetUploadRequest(request);
    }
};

template <typename FSM>
void HandleChunkResponse(
    uint32_t chunk_id,
    FSM & fsm,
    const std::string & url,
    const mf::http::Headers & headers,
    const std::string & content
)
{
    mf::api::ResponseBase response;
    response.InitializeWithContent(
        url,
        "",
        headers,
        content
        );

    if (response.error_code)
    {
        // Upload has unique negative values.
        fsm.ProcessEvent(event::Error{
            response.error_code,
            (response.error_string?*response.error_string:"Upload rejected")
            });
    }
    else
    {
        // We need the upload key to proceed.
        std::string upload_key;
        if ( mf::api::GetIfExists( response.pt, "response.doupload.key",
                &upload_key ) )
        {
            fsm.ProcessEvent( event::ChunkSuccess{chunk_id, upload_key});
        }
        else
        {
            int error = 0;
            if ( mf::api::GetIfExists( response.pt, "response.doupload.result",
                    &error ) )
            {
                fsm.ProcessEvent(event::Error{
                    std::error_code( error, upload_response_category() ),
                    "Upload rejected"
                    });
            }
            else
            {
                fsm.ProcessEvent(event::Error{
                    make_error_code(mf::api::api_code::ContentInvalidData),
                    "Upload response missing uploadkey"
                    });
            }
        }
    }
}

struct DoChunkUpload
{
    template <typename Event, typename FSM, typename SourceState,
             typename TargetState>
    void operator()(
            Event const &,
            FSM & fsm,
            SourceState&,
            TargetState&
        )
    {
        boost::optional<uint32_t> next_chunk = fsm.NextChunkToUpload();
        if (next_chunk)
        {
            UploadNextChunk(*next_chunk, fsm);
        }
        else
        {
            // Should only get here after getting upload key from previous
            // iterations.
            assert(!fsm.UploadKey().empty());
            fsm.ProcessEvent(event::ChunkUploadComplete{fsm.UploadKey()});
        }
    }

    template <typename FSM>
    std::string BuildUrl(
            uint32_t /*chunk_id*/,
            FSM & fsm
        )
    {
        std::string url = "http://www.mediafire.com/api/upload/resumable.php";

        std::map< std::string, std::string > query_map;

        // Action token
        query_map["session_token"] = fsm.ActionToken();

        // Use JSON
        query_map["response_format"] = "json";

        if (fsm.OnDuplicateAction() == OnDuplicateAction::Replace)
            query_map["action_on_duplicate"] = "replace";

        // Use JSON
        query_map["mtime"] = AsDateTime(fsm.Mtime());

        UploadTarget target_folder = fsm.TargetFolder();
        boost::apply_visitor(TargetUrlVisitor(&query_map), target_folder);

        /** @todo hjones: Query map overrides */

        return url + AssembleQuery(query_map);
    }

    template <typename FSM>
    void SetHeaders(
            mf::http::HttpRequest * http_request,
            uint32_t chunk_id,
            FSM & fsm
        )
    {
        assert(http_request);

        int begin, end;
        std::tie(begin, end) = fsm.ChunkRanges()[chunk_id];

        http_request->SetHeader("Content-Type", "application/octet-stream");

        // This is not required according to the documentation, but the server
        // is rejecting the upload without it.
        http_request->SetHeader("x-filename", fsm.Filename());

        { /* x-filesize */
            std::ostringstream ss;
            ss << fsm.Filesize();
            http_request->SetHeader("x-filesize", ss.str());
        }

        /* x-filehash */
        http_request->SetHeader("x-filehash", fsm.Hash());

        /** x-unit-hash */
        http_request->SetHeader("x-unit-hash", fsm.ChunkHashes()[chunk_id]);

        { /* x-unit-id */
            std::ostringstream ss;
            ss << chunk_id;
            http_request->SetHeader("x-unit-id", ss.str());
        }

        { /* x-unit-size */
            std::ostringstream ss;
            ss << (end-begin);
            http_request->SetHeader("x-unit-size", ss.str());
        }
    }

    template <typename FSM>
    void UploadNextChunk(
            uint32_t chunk_id,
            FSM & fsm
        )
    {
        auto ec = std::error_code();
        auto file_io = mf::utils::FileIO::Open(fsm.Path(), "r", &ec);
        if (ec)
        {
            fsm.ProcessEvent(event::Error{ec, "Unable to open file."});
            return;
        }

        int begin, end;
        std::tie(begin, end) = fsm.ChunkRanges()[chunk_id];

        file_io->Seek(mf::utils::SeekAnchor::Beginning, begin, &ec);

        assert(begin == file_io->Tell());

        if (ec)
        {
            fsm.ProcessEvent(event::Error{ec, "Seek file failed."});
            return;
        }

        // Add data to send as POST.
        auto pipe = std::make_shared<UploadPostDataPipe>(file_io, begin, end);
        auto url = BuildUrl(chunk_id, fsm);

        auto fsmp = fsm.AsFrontShared();
        auto request = mf::http::HttpRequest::Create(
            fsm.GetSessionMaintainer()->HttpConfig(),
            [fsmp, chunk_id, url](mf::http::HttpRequest::CallbackResponse response)
            {
                if (response.error_code)
                {
                    if (response.error_text)
                    {
                        fsmp->ProcessEvent(event::Error{
                            response.error_code,
                            *response.error_text
                            });
                    }
                    else
                    {
                        fsmp->ProcessEvent(event::Error{
                            response.error_code,
                            "Unknown error"
                            });
                    }
                }
                else
                {
                    HandleChunkResponse(chunk_id, *fsmp.get(), url,
                        response.headers, response.content);
                }
            },
            url);

        SetHeaders(request.get(), chunk_id, fsm);

        request->SetPostDataPipe(pipe);

        fsm.SetChunkState(chunk_id, ChunkState::Uploading);

        request->Start();

        fsm.SetUploadRequest(request);
    }
};

}  // namespace upload_transition
}  // namespace detail
}  // namespace uploader
}  // namespace mf
