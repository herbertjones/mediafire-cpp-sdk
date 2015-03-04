/**
 * @file api/actions/clone_cloud_tree.hpp
 * @author Herbert Jones
 * @brief Co-routine to
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include <utility>
#include <vector>

#include "boost/asio/coroutine.hpp"

#include "detail/coroutine.hpp"
#include "get_folder_content.hpp"

namespace mf
{
namespace api
{

/**
 * @class CloneCloudTreeVersioned
 * @brief Action to sync cloud directory hierarchy metadata.
 * @template RequestType The API folder/get_contents Request class to use.  Pass
 *                       in the class of the desired API version.
 *
 * This is the unversioned template.  Use GetFolderContent to use the current
 * API version.
 *
 * Create an object via the Create() static method.  The arguments are the same
 * as the constructor.
 */
template <typename RequestType>
class CloneCloudTreeVersioned
        : public detail::
                  ConcurrentCoroutine<CloneCloudTreeVersioned<RequestType>>
{
public:
    using Request = RequestType;
    using Response = typename Request::ResponseType;

    using Base = detail::ConcurrentCoroutine<CloneCloudTreeVersioned<Request>>;

    using Folder = typename Response::Folder;
    using File = typename Response::File;

    using FolderWorkList
            = std::vector<std::pair<std::string, FilesFoldersOrBoth>>;

    virtual ~CloneCloudTreeVersioned() {}

    /** Retrieved file data.  Pair first is parent folderkey.  Items are sorted
     * in order retrieved. */
    std::vector<std::pair<std::string, File>> files;

    /** Retrieved folder data.  Pair first is parent folderkey.  Items are
     * sorted in order retrieved. */
    std::vector<std::pair<std::string, Folder>> folders;

    /** When an error occurs, currently untraversed folders will end up here, so
     * that continued processing can be done. */
    FolderWorkList untraversed_folders;

    enum Defaults
    {
        /** Will by default try to make this many concurrent requests to the
         * API.  Use SetMaxConcurrent to override. */
        ConcurrentRequests = 10
    };

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
     * @param[in] folderkey The folderkey where to begin syncing.
     *
     * @return Return description
     */
    CloneCloudTreeVersioned(mf::api::SessionMaintainer * stm,
                            typename Base::Callback callback,
                            std::string folderkey);

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
     * @param[in] folderkeys Folderkey where to begin syncing.
     *
     * @return Return description
     */
    CloneCloudTreeVersioned(mf::api::SessionMaintainer * stm,
                            typename Base::Callback callback,
                            std::vector<std::string> folderkeys);

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
     * @param[in] folderkeys Folderkey where to begin syncing.
     *
     * @return Return description
     */
    CloneCloudTreeVersioned(mf::api::SessionMaintainer * stm,
                            typename Base::Callback callback,
                            FolderWorkList work_list);

    friend class detail::Coroutine<CloneCloudTreeVersioned>;
    friend class detail::ConcurrentCoroutine<CloneCloudTreeVersioned>;

    /**
     * @brief Internal callback.
     */
    void operator()();

    enum class ResponseResult
    {
        Success,
        ErrorHandled
    };
    ResponseResult HandleResponse();

    void EnqueueWork();

    std::vector<GetFolderContent::Pointer> folder_actions_;
    std::vector<GetFolderContent::Pointer> file_actions_;
    FolderWorkList folders_to_scan_;
};

/**
 * @class CloneCloudTree
 * @brief See CloneCloudTreeVersioned
 *
 * This is a convenience class that is always set to use the default API.
 */
class CloneCloudTree
        : public CloneCloudTreeVersioned<folder::get_content::Request>
{
};

}  // namespace api
}  // namespace mf

// Template members at bottom as they need definitions, but must be included.
#include "detail/clone_cloud_tree_impl.hpp"
