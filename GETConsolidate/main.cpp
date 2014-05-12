//
//  main.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/5/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "GETFrame.h"
#include "PadLookupTable.h"
#include "Event.h"
#include "GETDataFile.h"
#include "EventFile.h"
#include <exception>
#include <queue>
#include <vector>
#include <algorithm>

std::vector<boost::filesystem::path> FindGRAWFilesInDir(boost::filesystem::path eventRoot)
{
    if (!exists(eventRoot)) {
        throw 0;                 // FIX THIS
    }
    
    boost::filesystem::recursive_directory_iterator dirIter(eventRoot);
    boost::filesystem::recursive_directory_iterator endOfDir;
    std::vector<boost::filesystem::path> filesFound;
    
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
    
    try {
        filePaths = FindGRAWFilesInDir(input_path);
    }
    catch (int) {
        std::cout << "An error occurred." << std::endl;     // FIX!
    }
    if (filePaths.size() == 0) {
        std::cout << "No files found in that directory." << std::endl;
        //return 1;
        abort();        // Replace this with a throw
    }
    
    std::vector<GETDataFile> dataFiles;
    
    // Open all of the files
    
    for (auto filename : filePaths) {
        try {
            dataFiles.push_back( GETDataFile(filename.string()) );
        }
        catch (std::exception& e) {
            std::cout << "Exception thrown when opening file " << filename.string() << ": " << e.what() << std::endl;
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
                frames.push(GETFrame(file));
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
        
        // Write the event to the output file.
        
        output.WriteEvent(testEvent);
    }
    
    output.CloseFile();

}

int main(int argc, const char * argv[])
{
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
    
    boost::program_options::options_description opts_desc ("Allowed options.");

    
    opts_desc.add_options()
        ("help", "Output a help message")
        ("merge,m", "Merge input files")
        ("lookup,l", boost::program_options::value<boost::filesystem::path>(), "Lookup table")
        ("input,i", boost::program_options::value<boost::filesystem::path>(), "Input directory")
        ("output,o", boost::program_options::value<boost::filesystem::path>(), "Output file")
    ;
    
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, opts_desc), vm);
    boost::program_options::notify(vm);
    
    if (vm.count("help")) {
        std::cout << usage << std::endl;
        return 0;
    }

    if (vm.count("merge")) {
        if (vm.count("lookup") and vm.count("input") and vm.count("output")) {
            auto rootDir = vm["input"].as<boost::filesystem::path>();
            auto lookupTablePath = vm["lookup"].as<boost::filesystem::path>();
            auto outputFilePath = vm["output"].as<boost::filesystem::path>();
            
            MergeFiles(rootDir, outputFilePath, lookupTablePath);
        }
    }
    
    
    return 0;
}

