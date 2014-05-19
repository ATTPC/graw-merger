//
//  main.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/5/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include <iostream>
#include <iomanip>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "GETFrame.h"
#include "PadLookupTable.h"
#include "Event.h"
#include "GETDataFile.h"
#include "EventFile.h"
#include "GETExceptions.h"
#include <exception>
#include <queue>
#include <vector>
#include <algorithm>

bool g_verbose = false;

std::vector<boost::filesystem::path> FindGRAWFilesInDir(boost::filesystem::path eventRoot)
{
    namespace fs = boost::filesystem;
    
    if (!exists(eventRoot)) {
        throw Exceptions::Does_Not_Exist(eventRoot.string());
    }
    
    fs::recursive_directory_iterator dirIter(eventRoot);
    fs::recursive_directory_iterator endOfDir;
    std::vector<fs::path> filesFound;
    
    for ( ; dirIter != endOfDir; dirIter++) {
        if (is_directory(dirIter->path())) {
            if (g_verbose) {
                std::cout << "Entering directory: " << dirIter->path().string() << std::endl;
            }
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
    
    try {
        filePaths = FindGRAWFilesInDir(input_path);
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    if (filePaths.size() == 0) {
        std::cout << "No files found in that directory." << std::endl;
        throw Exceptions::Dir_is_Empty(input_path.string());
    }
    
    std::vector<GETDataFile> dataFiles;
    
    // Open all of the files
    
    for (auto filename : filePaths) {
        try {
            dataFiles.push_back(GETDataFile{filename, std::ios::in});
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }
    
    // Open the output file
    
    EventFile output;
    
    std::string output_path_string = output_path.string();
    
    if (is_directory(output_path)) {
        if (output_path_string.back() == '/') {
            output_path_string = output_path_string.append("output.bin");
        }
        else {
            output_path_string = output_path_string.append("/output.bin");
        }
    }
    
    output.OpenFileForWrite(output_path_string);

    
    // Main loop
    
    while (!dataFiles.empty()) {
        
        std::queue<GETFrame> frames;
        
        // Read in a frame from each file
        
        for (auto &file : dataFiles) {
            try {
                std::vector<uint8_t> raw_frame = file.ReadRawFrame();
                frames.push(GETFrame {raw_frame, file.GetFileCobo(), file.GetFileAsad()} );
            }
            catch (std::exception& e) {
                std::cout << e.what() << std::endl;
            }
        }
        
        // Go through the files and get rid of eofs
        
        dataFiles.erase(std::remove_if(dataFiles.begin(), dataFiles.end(),
                                       [] (const GETDataFile & x) {return x.eof();}),
                        dataFiles.end());
        
        // Create an event from this set of frames, if there are any.
        
        if (frames.empty()) break;
        
        Event testEvent;
        testEvent.SetLookupTable(&lookupTable);
        
        while (!frames.empty()) {
            testEvent.AppendFrame(frames.front());
            frames.pop();
            //std::cout << "Appended frame." << std::endl;
        }
        
        testEvent.SubtractFPN();
        
        // Write the event to the output file.
        
        output.WriteEvent(testEvent);
    }
    
    output.CloseFile();

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
    
    /* Arguments passed from command line:
     *     [command] [verb] [options] [input directory] [output file]
     */
    
    // Usage:
    
    std::string usage = "Usage:\n"
        "[command] [verb] [options] [inputs] [outputs] \n"
        "Verbs include: merge, info\n"
        "See documentation for more information.";
    
    std::string merge_usage = "Verb 'merge' usage:\n"
        "[command] merge [options] input_directory output_file\n"
        "The input directory must have the correct structure.";
    
    po::options_description opts_desc ("Allowed options.");

    
    opts_desc.add_options()
        ("help", "Output a help message")
        ("merge,m", "Merge input files")
        ("ls,l", "List event file contents")
        ("lookup,l", po::value<fs::path>(), "Lookup table")
        ("input,i", po::value<fs::path>(), "Input directory")
        ("output,o", po::value<fs::path>(), "Output file")
        ("verbose,v", "Print more output.")
    ;
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, opts_desc), vm);
    po::notify(vm);
    
    if (vm.count("help")) {
        std::cout << usage << std::endl;
        return 0;
    }
    
    if (vm.count("verbose")) {
        g_verbose = true;
    }

    if (vm.count("merge")) {
        if (vm.count("lookup") and vm.count("input") and vm.count("output")) {
            auto rootDir = vm["input"].as<fs::path>();
            auto lookupTablePath = vm["lookup"].as<fs::path>();
            auto outputFilePath = vm["output"].as<fs::path>();
            
            MergeFiles(rootDir, outputFilePath, lookupTablePath);
        }
    }
    
    if (vm.count("ls")) {
        auto filePath = vm["input"].as<fs::path>();
        ListEventFileContents(filePath);
    }
    
    
    return 0;
}

