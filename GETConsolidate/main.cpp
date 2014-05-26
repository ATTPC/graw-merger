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
#include "UserInterface.h"

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
    
    std::cout << "Looking for files." << std::endl;
    
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
    
    // This could throw if the dir is empty, but I can't do anything about it
    // here, so let the caller catch it.
    
    filePaths = FindGRAWFilesInDir(input_path);

    if (filePaths.size() == 0) {
        throw Exceptions::Dir_is_Empty(input_path.string());
    }
    
    std::vector<GETDataFile> dataFiles;
    
    // Open all of the files
    
    for (auto filename : filePaths) {
        try {
            dataFiles.push_back(GETDataFile{filename, std::ios::in});
        }
        catch (std::exception& e) {
            UI::Log(UI::LogLevel::Error, e.what());
        }
    }
    
    // Open the output file
    
    EventFile output;
//    GETDataFile output;
    
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
    
    UI::ProgressBar prog {};
    
    long unsigned total_size = 0;
    
    for (auto item : filePaths) {
        total_size += boost::filesystem::file_size(item);
    }
    
    while (!dataFiles.empty()) {
        // Find progress
        
        long unsigned total_pos = 0;
        
        for (auto &file : dataFiles) {
            total_pos += file.GetPosition();
        }
        
        prog.SetPercent(static_cast<int>(100*total_pos / total_size));
        prog.Write();
        
        std::queue<GETFrame> frames;
        
        // Read in a frame from each file
        
        for (auto &file : dataFiles) {
            try {
                std::vector<uint8_t> raw_frame = file.ReadRawFrame();
                frames.push(GETFrame {raw_frame, file.GetFileCobo(), file.GetFileAsad()} );
            }
            catch (std::exception& e) {
                UI::Log(UI::LogLevel::Error, e.what());
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
            UI::Log(UI::LogLevel::Debug, "Appended frame.");
        }
        
        testEvent.SubtractFPN();
        
        // Write the event to the output file.
        
        output.WriteEvent(testEvent);
        
//        auto procFrames = testEvent.ExtractAllFrames();
//        
//        for (auto fr : procFrames) {
//            output.WriteFrame(fr);
//        }
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

    po::variables_map vm;
    
    opts_desc.add_options()
        ("help", "Output a help message")
        ("merge,m", "Merge input files")
        ("frames,f", "Write output as frames")
        ("ls,l", "List event file contents")
        ("lookup,l", po::value<fs::path>(), "Lookup table")
        ("input,i", po::value<fs::path>(), "Input directory")
        ("output,o", po::value<fs::path>(), "Output file")
        ("verbose,v", "Print more output.")
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
    
    if (vm.count("verbose")) {
        g_verbose = true;
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
    
    if (vm.count("ls")) {
        auto filePath = vm["input"].as<fs::path>();
        ListEventFileContents(filePath);
    }
    
    
    return 0;
}

