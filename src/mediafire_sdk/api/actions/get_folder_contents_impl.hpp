namespace mf
{
namespace api
{

template <class TRequest>
GetFolderContents<TRequest>::ErrorType::ErrorType(
        const std::string & folder_key,
        FilesOrFoldersOrBoth files_or_folders_or_both,
        uint32_t chunk_num,
        const std::error_code & error_code,
        const std::string & error_string)
        : folder_key(folder_key),
          files_or_folders_or_both(files_or_folders_or_both),
          chunk_num(chunk_num),
          error_code(error_code),
          error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<GetFolderContents<TRequest>>
GetFolderContents<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & folder_key,
        const FilesOrFoldersOrBoth & files_or_folders_or_both,
        CallbackType && callback)
{
    return std::shared_ptr<GetFolderContents>(new GetFolderContents(
            stm, folder_key, files_or_folders_or_both, std::move(callback)));
}

template <class TRequest>
void GetFolderContents<TRequest>::operator()()
{
    coro_();
}

template <class TRequest>
GetFolderContents<TRequest>::GetFolderContents(
        SessionMaintainer * stm,
        const std::string & folder_key,
        const FilesOrFoldersOrBoth & files_or_folders_or_both,
        CallbackType && callback)
        : stm_(stm),
          folder_key_(folder_key),
          files_or_folders_or_both_(files_or_folders_or_both),
          callback_(callback)
{
    if (files_or_folders_or_both_ == FilesOrFoldersOrBoth::Files
        || files_or_folders_or_both_ == FilesOrFoldersOrBoth::Both)
    {
        file_chunks_remaining_ = true;
    }

    if (files_or_folders_or_both_ == FilesOrFoldersOrBoth::Folders
        || files_or_folders_or_both_ == FilesOrFoldersOrBoth::Both)
    {
        folder_chunks_remaining_ = true;
    }
}
}  // namespace mf
}  // namespace api