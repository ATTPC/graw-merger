//
//  main.cpp
//  get-manip
//
//  Created by Joshua Bradt on 5/5/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include <iostream>
#include <iomanip>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "GRAWFrame.h"
#include "PadLookupTable.h"
#include "LookupTable.h"
#include "Event.h"
#include "GRAWFile.h"
#include "EventFile.h"
#include "GMExceptions.h"
#include <exception>
#include <queue>
#include <vector>
#include <algorithm>
#include "UserInterface.h"
#include "Merger.h"
#include "Constants.h"

std::vector<boost::filesystem::path> FindGRAWFilesInDir(boost::filesystem::path eventRoot)
{
    namespace fs = boost::filesystem;
    
    if (!exists(eventRoot)) {
        throw Exceptions::Does_Not_Exist(eventRoot.string());
    }
    
    fs::recursive_directory_iterator dirIter(eventRoot);
    fs::recursive_directory_iterator endOfDir;
    std::vector<fs::path> filesFound;
    
    std::cout << "Looking for files." << std::endl;
    
    for ( ; dirIter != endOfDir; dirIter++) {
        if (is_directory(dirIter->path())) {
            std::cout << "Entering directory: " << dirIter->path().string() << std::endl;
        }
        else if ((boost::filesystem::is_regular_file(dirIter->path()) ||
                  boost::filesystem::is_symlink(dirIter->path()))
                 && dirIter->path().extension() == ".graw") {
            auto resolved_path = boost::filesystem::canonical(dirIter->path());
            std::cout << "    Found file: " << resolved_path.filename().string() << std::endl;
            filesFound.push_back(resolved_path);
        }
    }
    
    std::cout << "Found " << filesFound.size() << " GRAW files." << std::endl;
    
    return filesFound;
}

void MergeFiles(boost::filesystem::path input_path,
                boost::filesystem::path output_path,
                boost::filesystem::path lookup_path,
                LookupTable<sample_t>& pedsTable,
                bool suppZeros, sample_t threshold)
{
    // Import the lookup table
    
    PadLookupTable lookupTable (lookup_path.string());
    
    // Find files in the provided directory
    
    std::vector<boost::filesystem::path> filePaths;
    
    // This could throw if the dir is empty, but I can't do anything about it
    // here, so let the caller catch it.
    
    filePaths = FindGRAWFilesInDir(input_path);

    if (filePaths.size() == 0) {
        throw Exceptions::Dir_is_Empty(input_path.string());
    }
    
    Merger mg;
    
    // Add the files to the merger
    
    std::cout << "Mapping frames in files..." << std::endl;
    
    for (const auto& path : filePaths) {
        int numFound = mg.AddFramesFromFileToIndex(path);
        std::cout << "    Found " << numFound << " frames in file " << path.filename().string() << std::endl;
    }
    
    std::cout << "Finished mapping files." << std::endl;
    
    std::string output_path_string = output_path.string();
    
    mg.MergeByEvtId(output_path_string, &lookupTable, pedsTable,
                    suppZeros, threshold);
    
    std::cout << '\n' << "Finished merging files." << std::endl;
}

int main(int argc, const char * argv[])
{
    namespace po = boost::program_options;
    namespace fs = boost::filesystem;
        
    // Usage:
    
    std::string usage =
        "get-manip (v1.4.1): A tool for merging GRAW files into Event files.\n"
        "\n"
        "usage: get-manip --lookup <path> [--pedestals <path>] [--threshold <value>]\n"
        "                [--zerosupp] <input_path> [<output_path>]\n"
        "\n"
        "If output file is not specified, default is based on input path.\n"
        "Ex: /data/run_0001/ as input produces /data/run_0001.evt as output.";
    
    po::options_description opts_desc ("Allowed options.");

    po::variables_map vm;
    
    opts_desc.add_options()
        ("help,h", "Output a help message")
        ("lookup,l", po::value<fs::path>(), "Lookup table")
        ("pedestals,p", po::value<fs::path>(), "Pedestals file")
        ("threshold,t", po::value<sample_t>(), "Threshold")
        ("zerosupp,z", po::bool_switch(), "Zero suppression")
        ("input,i", po::value<fs::path>(), "Input directory")
        ("output,o", po::value<fs::path>(), "Output file")
    ;
    
    po::positional_options_description pos_opts;
    pos_opts.add("input", 1);
    pos_opts.add("output", 1);
    
    po::store(po::parse_command_line(argc, argv, opts_desc), vm);
    
    po::store(po::command_line_parser(argc, argv).options(opts_desc).positional(pos_opts).run(), vm);

    po::notify(vm);
    
    if (vm.count("help")) {
        std::cout << usage << std::endl;
        return 0;
    }
    
    if (vm.count("lookup") and vm.count("input")) {
        // This is the typical execution path
        
        auto rootDir = vm["input"].as<fs::path>();
        if (not fs::exists(rootDir)) {
            std::cout << "Error: Provided input path does not exist." << std::endl;
            return 1;
        }
        
        auto lookupTablePath = vm["lookup"].as<fs::path>();
        if (not fs::exists(lookupTablePath)) {
            std::cout << "Error: Provided lookup table path does not exist." << std::endl;
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
                outputFilePathString.append(".evt");
                outputFilePath = fs::path {outputFilePathString};
            }
            else {
                std::string outputFilePathString = rootDir.string() + ".evt";
                outputFilePath = fs::path {outputFilePathString};
            }
        }
        
        // Load the pedestals file, if given
        
        auto pedsTable = LookupTable<sample_t> {};
        if (vm.count("pedestals")) {
            // Load pedestals
            auto pedsPath = vm["pedestals"].as<fs::path>();
            if (fs::exists(pedsPath)) {
                pedsTable.ReadFile(pedsPath.string());
            }
            else {
                std::cout << "Error: Pedestals file does not exist." << std::endl;
                return 1;
            }
        }
        
        bool suppZeros {false};
        if (vm.count("zerosupp")) {
            suppZeros = true;
        }
        
        // Init threshold to min sample value: like not having a threshold
        sample_t threshold {Constants::min_sample};
        if (vm.count("threshold")) {
            threshold = vm["threshold"].as<sample_t>();
        }
        
        try {
            MergeFiles(rootDir, outputFilePath, lookupTablePath, pedsTable,
                       suppZeros, threshold);
        }
        catch (std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
            return 1;
        }
    }
    else {
        std::cout << usage << std::endl;
        return 1;
    }
    
    return 0;
}

