/**
 * @file upload_file.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include <cstdlib>
#include <iostream>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <memory>

#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif
#include "boost/bind.hpp"
#include "boost/filesystem.hpp"
#include "boost/optional.hpp"
#include "boost/program_options.hpp"
#include "boost/variant/apply_visitor.hpp"
#include "boost/variant/get.hpp"

#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/uploader/upload_manager.hpp"

namespace po = boost::program_options;
namespace asio = boost::asio;
namespace us = mf::uploader::upload_state;

class StatusVisitor : public boost::static_visitor<>
{
public:
    StatusVisitor(asio::io_service & io_service,
                  int id,
                  const int files_expected,
                  int * files_complete,
                  std::map<int, std::string> * summary_map)
            : io_service_(io_service),
              files_expected_(files_expected),
              files_complete_(files_complete),
              summary_map_(summary_map),
              id_(id)
    {
        if (files_expected > 0)
        {
            std::ostringstream ss;
            ss << "[" << (id + 1) << "] ";
            id_str_ = ss.str();
        }
    }

    void operator()(us::EnqueuedForHashing &) const {}

    void operator()(us::Hashing &) const
    {
        std::cout << id_str_ << "Hashing file." << std::endl;
    }

    void operator()(us::EnqueuedForUpload &) const {}

    void operator()(us::Uploading & status) const
    {
        std::cout << id_str_ << "Upload started." << std::endl;
    }

    void operator()(us::Polling &) const
    {
        std::cout << id_str_ << "Polling server for file key." << std::endl;
    }

    void operator()(us::Error & status) const
    {
        const auto & ec = status.error_code;
        std::cout << id_str_ << "Error: " << ec.message() << std::endl;
        AddDebug("Received error: " + ec.message());
        AddDebug("Error type: " + std::string(ec.category().name()));
        AddDebug("Error value: " + mf::utils::to_string(ec.value()));
        AddDebug("Description: " + status.description);

        *files_complete_ += 1;
        if (*files_complete_ == files_expected_)
            io_service_.stop();
    }

    void operator()(us::Complete & status) const
    {
        std::cout << id_str_ << "Upload complete." << std::endl;
        AddDebug("Upload complete.");
        AddDebug("New quickkey: " + status.quickkey);
        AddDebug("Filename: " + status.filename);

        *files_complete_ += 1;
        if (*files_complete_ == files_expected_)
            io_service_.stop();
    }

private:
    void AddDebug(std::string str) const { (*summary_map_)[id_] += str + "\n"; }

    asio::io_service & io_service_;
    const int files_expected_;
    int * files_complete_;
    std::map<int, std::string> * summary_map_;

    int id_;
    std::string id_str_;
};

void ShowUsage(const char * filename, const po::options_description & visible)
{
    std::cout << "Usage: " << filename << " [options]"
                                          " -u USERNAME"
                                          " -p PASSWORD"
                                          " FILES\n";
    std::cout << visible << "\n";
}

int main(int argc, char * argv[])
{
    try
    {
        std::string directory_path;
        std::string folderkey;
        std::string password;
        std::string save_as;
        std::vector<std::string> upload_file_path;
        std::string username;

        po::options_description visible("Allowed options");
        /* clang-format off */
        visible.add_options()
            ("folderkey"     , po::value<std::string>(&folderkey)      , "Folderkey to the directory where to upload")
            ("help,h"        ,                                           "Show this message.")
            ("password,p"    , po::value<std::string>(&password)       , "Password for login")
            ("path"          , po::value<std::string>(&directory_path) , "Directory path where to upload file")
            ("saveas,s"      , po::value<std::string>(&save_as)        , "Upload file with custom name.  If multiple files passed, only the first is renamed.")
            ("username,u"    , po::value<std::string>(&username)       , "Username for login")
            ("replace,r"     ,                                           "Replace file if one exists already with the same name.")
            ("autorename,a"  ,                                           "Rename the file if it exists already.")
            ;
        /* clang-format on */

        po::options_description hidden("Hidden options");
        hidden.add_options()(
                "upload_file_path",
                po::value<std::vector<std::string>>(&upload_file_path),
                "upload_file_path");

        po::positional_options_description p;
        p.add("upload_file_path", -1);

        po::options_description cmdline_options;
        cmdline_options.add(visible).add(hidden);

        po::variables_map vm;
        try
        {
            po::store(po::command_line_parser(argc, argv)
                              .options(cmdline_options)
                              .positional(p)
                              .run(),
                      vm);
            po::notify(vm);
        }
        catch (boost::program_options::error & err)
        {
            std::cout << "Error: " << err.what() << std::endl;
            ShowUsage(argv[0], visible);
            return 1;
        }

        if (vm.count("help") || !vm.count("upload_file_path")
            || !vm.count("username") || !vm.count("password"))
        {
            ShowUsage(argv[0], visible);
            return 0;
        }

        if (vm.count("replace") && vm.count("autorename"))
        {
            std::cout << "Unable to replace and autorename." << std::endl;
            ShowUsage(argv[0], visible);
            return 1;
        }

        asio::io_service io_service;

        std::map<int, std::string> summary_map;

        {
            auto http_config = mf::http::HttpConfig::Create();
            http_config->SetWorkIoService(&io_service);

            mf::api::SessionMaintainer stm(http_config);

            // Handle session token failures.
            stm.SetSessionStateChangeCallback(
                    [&io_service](mf::api::SessionState state)
                    {
                        if (boost::get<mf::api::session_state::
                                               CredentialsFailure>(&state))
                        {
                            std::cout << "Username or password incorrect."
                                      << std::endl;
                            io_service.stop();
                        }
                    });

            stm.SetLoginCredentials(
                    mf::api::credentials::Email{username, password});

            mf::uploader::UploadManager um(&stm);

            const int files_to_upload = upload_file_path.size();
            int files_uploaded = 0;
            for (auto file_id = 0; file_id < files_to_upload; ++file_id)
            {
                mf::uploader::UploadRequest request(upload_file_path[file_id]);

                if (vm.count("folderkey"))
                    request.SetTargetFolderkey(folderkey);

                if (vm.count("path"))
                    request.SetTargetFolderPath(directory_path);

                if (vm.count("saveas") && file_id == 0)
                    request.SetTargetFilename(save_as);

                if (vm.count("replace"))
                {
                    request.SetOnDuplicateAction(
                            mf::uploader::OnDuplicateAction::Replace);
                }

                if (vm.count("autorename"))
                {
                    request.SetOnDuplicateAction(
                            mf::uploader::OnDuplicateAction::AutoRename);
                }

                um.Add(request,
                       [&io_service, files_to_upload, &files_uploaded, file_id,
                        &summary_map](mf::uploader::UploadStatus status)
                       {
                           boost::apply_visitor(
                                   StatusVisitor(io_service, file_id,
                                                 files_to_upload,
                                                 &files_uploaded, &summary_map),
                                   status.state);
                       });
            }

            io_service.run();

            if (files_to_upload > 0)
            {
                int file_id = 0;
                std::cout << std::endl;
                for (auto & pair : summary_map)
                {
                    std::cout << "File " << (file_id + 1) << ": "
                              << upload_file_path[file_id] << std::endl;
                    std::cout << pair.second << std::endl;
                    ++file_id;
                }
            }
        }
    }
    catch (std::exception & e)
    {
        std::cerr << "Uncaught exception: " << e.what() << "\n";
        return 1;
    }
    catch (...)
    {
        std::cerr << "Exception of unknown type!\n";
        return 1;
    }

    return 0;
}
