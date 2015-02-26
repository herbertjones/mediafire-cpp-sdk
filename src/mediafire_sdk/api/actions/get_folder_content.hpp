/**
 * @file api/actions/get_folder_content.hpp
 * @author Herbert Jones
 * @brief Action to get all the children from a folder.
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include <memory>
#include <vector>

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/folder/get_content.hpp"

#include "detail/concurrent_coroutine.hpp"

namespace mf
{
namespace api
{

/** Input to GetFolderContent.  Determines what type of data is to be
 * collected.*/
enum class GetFolderContentType
{
    Folders,
    Files,
    FilesAndFolders
};

/**
 * @class GetFolderContentVersioned
 * @brief Action to retrieve cloud directory hierarchy metadata from a single
 *        directory.
 * @template RequestType The API folder/get_contents Request class to use.  Pass
 *                       in the class of the desired API version.
 *
 * This is the unversioned template.  Use CloneCloudTree to use the current API
 * version.
 *
 * Useful, as this encapsulates the logic required to get all children of a
 * folder without having to make repeated folder/get_content calls.
 *
 * Create an object via the Create() static method.  The arguments are the same
 * as the constructor.
 */
template <typename RequestType>
class GetFolderContentVersioned
        : public detail::
                  ConcurrentCoroutine<GetFolderContentVersioned<RequestType>>
{
public:
    using Request = RequestType;
    using Response = typename Request::ResponseType;

    using Base
            = detail::ConcurrentCoroutine<GetFolderContentVersioned<Request>>;

    using Folder = typename Response::Folder;
    using File = typename Response::File;

    virtual ~GetFolderContentVersioned() {}

    /** Retrieved file data.  Items are sorted in order retrieved. */
    std::vector<File> files;

    /** Retrieved folder data.  Items are sorted in order retrieved. */
    std::vector<Folder> folders;

    /** The folderkey of the directory being traversed. */
    std::string Folderkey() const { return folderkey_; }

protected:
    /**
     * @brief Constructor
     *
     * This constructor creates an object.  As it is protected, the Create
     * static method must be used.  Its arguments are the same as this, but
     * returns a shared pointer.  Once started, the shared object will be kept
     * alive until the callback returns success or failure, or the io_service is
     * destroyed.
     *
     * @param[in] stm Session manager
     * @param[in] callback This is called with the result of the operation.
     *                     Its signature is:
     *                     void(ActionResult, Pointer)
     * @param[in] folderkey The folderkey to scan.
     * @param[in] type Determines if files, folders, or both will be gathered.
     */
    GetFolderContentVersioned(mf::api::SessionMaintainer * stm,
                              typename Base::Callback callback,
                              std::string folderkey,
                              GetFolderContentType type);

    friend class detail::Coroutine<GetFolderContentVersioned<Request>>;
    friend class detail::
            ConcurrentCoroutine<GetFolderContentVersioned<Request>>;

    /**
     * @brief Internal callback.
     */
    void operator()();

    bool EnqueueFileRequest();
    bool EnqueueFolderRequest();

    enum class HandledFailure
    {
        Yes,
        No
    };
    HandledFailure HandleFileResponse();
    HandledFailure HandleFolderResponse();

    const std::string folderkey_;
    const GetFolderContentType type_;

    uint32_t chunk_number_;

    std::unique_ptr<Request> current_folder_request_;
    Response folder_response_;

    std::unique_ptr<Request> current_file_request_;
    Response file_response_;

    bool file_request_remaining_;
    bool folder_request_remaining_;
};

/**
 * @class GetFolderContent
 * @brief See GetFolderContentVersioned
 *
 * This is a convenience class that is always set to use the default API.
 */
class GetFolderContent
        : public GetFolderContentVersioned<folder::get_content::Request>
{
};

}  // namespace api
}  // namespace mf

// Template members at bottom as they need definitions, but must be included.
#include "detail/get_folder_content_impl.hpp"
