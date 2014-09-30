/**
 * @file hash_file.cpp
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

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#  include "boost/asio/impl/src.hpp"  // Define once in program
#  include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif
#include "boost/bind.hpp"
#include "boost/filesystem.hpp"
#include "boost/optional.hpp"
#include "boost/program_options.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"

#include "mediafire_sdk/uploader/hasher.hpp"

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    try {
        std::string filepath;

        po::options_description visible("Allowed options");
        visible.add_options()
            ("help,h", "Show this message.");

        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("file", po::value<std::string>(&filepath), "file");


        po::positional_options_description p;
        p.add("file", 1);

        po::options_description cmdline_options;
        cmdline_options.add(visible).add(hidden);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
                options(cmdline_options).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help") || ! vm.count("file"))
        {
            std::cout << "Usage: " << argv[0] << " [options] FILE\n";
            std::cout << visible << "\n";
            return 0;
        }

        mf::uploader::Hasher::Callback callback([](
                    std::error_code ec,
                    boost::optional<mf::uploader::FileHashes> file_hashes
                )
            {
                if (ec)
                {
                    std::cout << "Got error: " << ec.message() << std::endl;
                }
                else if (! file_hashes)
                {
                    std::cout << "Error: No hash!" << std::endl;
                    assert(!"No hash and no error!");
                }
                else
                {
                    std::cout << "Hash: " << file_hashes->hash << std::endl;

                    std::cout << "Chunk hashes: " << std::endl;
                    for (const auto & hash: file_hashes->chunk_hashes)
                    {
                        std::cout << "    " << hash << std::endl;
                    }

                    std::cout << "Chunk ranges: " << std::endl;
                    for (const auto & range: file_hashes->chunk_ranges)
                    {
                        std::cout << "    [" << range.first << ", "
                            << range.second << ")" << std::endl;
                    }
                }
            });

        boost::filesystem::path path(filepath);

        // Get mtime
        const std::time_t mtime = boost::filesystem::last_write_time(path);

        // Get filesize
        const uint64_t filesize = boost::filesystem::file_size(path);

        boost::asio::io_service io_service;

        auto hasher = mf::uploader::Hasher::Create(
            &io_service,
            path,
            filesize,
            mtime,
            callback);

        std::cout << "Starting hashing..." << std::endl;
        hasher->Start();
        io_service.run();
    }
    catch(std::exception& e)
    {
        std::cerr << "Uncaught exception: " << e.what() << "\n";
        return 1;
    }
    catch(...)
    {
        std::cerr << "Exception of unknown type!\n";
        return 1;
    }

    return 0;
}
