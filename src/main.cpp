#include <iostream>
#include <iomanip>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/program_options.hpp>
#include "GRAWFrame.h"
#include "PadLookupTable.h"
#include "LookupTable.h"
#include "Event.h"
#include "GRAWFile.h"
#include "GMExceptions.h"
#include <exception>
#include <queue>
#include <vector>
#include <algorithm>
#include "Merger.h"
#include "Constants.h"

std::vector<std::string> FindGRAWFilesInDir(boost::filesystem::path eventRoot)
{
    namespace fs = boost::filesystem;

    if (!exists(eventRoot)) {
        throw Exceptions::Does_Not_Exist(eventRoot.string());
    }

    fs::recursive_directory_iterator dirIter(eventRoot);
    fs::recursive_directory_iterator endOfDir;
    std::vector<std::string> filesFound;

    BOOST_LOG_TRIVIAL(info) << "Looking for files";

    for ( ; dirIter != endOfDir; dirIter++) {
        if (is_directory(dirIter->path())) {
            BOOST_LOG_TRIVIAL(info) << "Entering directory: " << dirIter->path().string();
        }
        else if ((boost::filesystem::is_regular_file(dirIter->path()) ||
                  boost::filesystem::is_symlink(dirIter->path()))
                 && dirIter->path().extension() == ".graw") {
            auto resolved_path = boost::filesystem::canonical(dirIter->path());
            BOOST_LOG_TRIVIAL(info) << "Found file: " << resolved_path.filename().string();
            filesFound.push_back(resolved_path.string());
        }
    }

    BOOST_LOG_TRIVIAL(info) << "Found " << filesFound.size() << " GRAW files";

    return filesFound;
}

void MergeFiles(boost::filesystem::path input_path,
                boost::filesystem::path output_path,
                boost::filesystem::path lookup_path)
{
    // Import the lookup table

    std::shared_ptr<PadLookupTable> lookupTable = std::make_shared<PadLookupTable>(lookup_path.string());

    // Find files in the provided directory

    std::vector<std::string> filePaths = FindGRAWFilesInDir(input_path);


    if (filePaths.size() == 0) {
        throw Exceptions::Dir_is_Empty(input_path.string());
    }

    Merger mg (filePaths, lookupTable);

    mg.MergeByEvtId(output_path.string());

    BOOST_LOG_TRIVIAL(info) << "Finished merging files.";
}

int main(int argc, const char * argv[])
{
    namespace po = boost::program_options;
    namespace fs = boost::filesystem;

    // Usage:

    std::string usage =
        "graw2hdf (v2.0): A tool for merging GRAW files into HDF5 files.\n"
        "\n"
        "usage: graw2hdf [-v] --lookup <path> <input_path> [<output_path>]\n"
        "\n"
        "If output file is not specified, default is based on input path.\n"
        "Ex: /data/run_0001/ as input produces /data/run_0001.h5 as output.";

    po::options_description opts_desc ("Allowed options.");

    po::variables_map vm;

    opts_desc.add_options()
        ("help,h", "Output a help message")
        ("verbose,v", "Show more output")
        ("lookup,l", po::value<fs::path>(), "Lookup table")
        ("input,i", po::value<fs::path>(), "Input directory")
        ("output,o", po::value<fs::path>(), "Output file")
    ;

    po::positional_options_description pos_opts;
    pos_opts.add("input", 1);
    pos_opts.add("output", 1);

    try {
        po::store(po::parse_command_line(argc, argv, opts_desc), vm);
        po::store(po::command_line_parser(argc, argv).options(opts_desc).positional(pos_opts).run(), vm);
    }
    catch (po::error& e) {
        BOOST_LOG_TRIVIAL(fatal) << "Error parsing command line: " << e.what();
        std::cout << usage << std::endl;
        return 1;
    }

    po::notify(vm);

    if (vm.count("help")) {
        std::cout << usage << std::endl;
        return 0;
    }

    if (!vm.count("verbose")) {
        boost::log::core::get()->set_filter(
            boost::log::trivial::severity >= boost::log::trivial::info
        );
    }

    if (vm.count("lookup") and vm.count("input")) {
        // This is the typical execution path

        auto rootDir = vm["input"].as<fs::path>();
        if (not fs::exists(rootDir)) {
            BOOST_LOG_TRIVIAL(fatal) << "Error: Provided input path does not exist.";
            return 1;
        }

        auto lookupTablePath = vm["lookup"].as<fs::path>();
        if (not fs::exists(lookupTablePath)) {
            BOOST_LOG_TRIVIAL(fatal) << "Error: Provided lookup table path does not exist.";
            return 1;
        }

        // Build the output path
        fs::path outputFilePath {};
        if (vm.count("output")) {
            outputFilePath = vm["output"].as<fs::path>();
        }
        else {
            if (rootDir.string().back() == '/') {
                std::string outputFilePathString = rootDir.string();
                outputFilePathString.pop_back();
                outputFilePathString.append(".h5");
                outputFilePath = fs::path {outputFilePathString};
            }
            else {
                std::string outputFilePathString = rootDir.string() + ".h5";
                outputFilePath = fs::path {outputFilePathString};
            }
        }

        try {
            MergeFiles(rootDir, outputFilePath, lookupTablePath);
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(fatal) << "Error: " << e.what();
            return 1;
        }
    }
    else {
        std::cout << usage << std::endl;
        return 0;
    }

    return 0;
}
