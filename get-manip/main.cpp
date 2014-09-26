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
        else if (is_regular_file(dirIter->path()) && dirIter->path().extension() == ".graw") {
            std::cout << "    Found file: " << dirIter->path().filename().string() << std::endl;
            filesFound.push_back(dirIter->path());
        }
    }
    
    std::cout << "Found " << filesFound.size() << " GRAW files." << std::endl;
    
    return filesFound;
}

void MergeFiles(boost::filesystem::path input_path,
                boost::filesystem::path output_path,
                boost::filesystem::path lookup_path)
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
    
    if (is_directory(output_path)) {
        if (output_path_string.back() == '/') {
            output_path_string = output_path_string.append("output.bin");
        }
        else {
            output_path_string = output_path_string.append("/output.bin");
        }
    }
    
    mg.MergeByEvtId(output_path_string, &lookupTable);
    
    std::cout << '\n' << "Finished merging files." << std::endl;
}

void ListEventFileContents(boost::filesystem::path filepath)
{
    namespace fs = boost::filesystem;
    // Test the path
    if (!fs::is_regular_file(filepath)) throw Exceptions::Bad_File(filepath.string());
    
    EventFile efile {};
    efile.OpenFileForRead(filepath.string());
    
    std::cout << std::setw(4) << "ID" << std::setw(13) << "Time" << std::endl;
    std::cout << "-----------------" << std::endl;
    
    while (!efile.eof()) {
        try {
            Event thisEvent = efile.GetNextEvent();
            
            auto id = thisEvent.GetEventId();
            auto time = thisEvent.GetEventTime();
        
            std::cout << std::setw(4) << id << std::setw(13) << time << std::endl;
            
        }
        catch (Exceptions::End_of_File& eof) {
            break;
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }
    
    efile.CloseFile();
}

int main(int argc, const char * argv[])
{
    namespace po = boost::program_options;
    namespace fs = boost::filesystem;
        
    // Usage:
    
    std::string usage = "Usage:\n"
        "get-manip [verb] [inputs] [outputs] \n"
        "Verbs include: merge, ls\n"
        "See documentation for more information.";
    
    std::string merge_usage = "Verb 'merge' usage:\n"
        "get-manip --merge [options] input_directory output_file\n"
        "The input directory must have the correct structure.";
    
    po::options_description opts_desc ("Allowed options.");

    po::variables_map vm;
    
    opts_desc.add_options()
        ("help", "Output a help message")
        ("merge,m", "Merge input files")
        ("frames,f", "Write output as frames")
        ("ls,l", "List event file contents")
        ("lookup,l", po::value<fs::path>(), "Lookup table")
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
    
    if (vm.count("merge")) {
        if (vm.count("lookup") and vm.count("input") and vm.count("output")) {
            auto rootDir = vm["input"].as<fs::path>();
            auto lookupTablePath = vm["lookup"].as<fs::path>();
            auto outputFilePath = vm["output"].as<fs::path>();
            
            try {
                MergeFiles(rootDir, outputFilePath, lookupTablePath);
            }
            catch (std::exception& e) {
                std::cout << "Error: " << e.what() << std::endl;
                return 1;
            }
        }
    }
    else if (vm.count("ls")) {
        auto filePath = vm["input"].as<fs::path>();
        ListEventFileContents(filePath);
    }
    else {
        std::cout << usage << std::endl;
    }
    
    
    return 0;
}

