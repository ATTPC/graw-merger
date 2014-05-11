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
#include "GETFrame.h"
#include "PadLookupTable.h"
#include "Event.h"
#include "GETDataFile.h"
#include "EventFile.h"
#include <exception>
#include <queue>
#include <vector>
#include <algorithm>

std::vector<boost::filesystem::path> FindFilesInEventDir(boost::filesystem::path eventRoot)
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

int main(int argc, const char * argv[])
{
    boost::filesystem::path rootDir (argv[1]); // check these
    boost::filesystem::path lookupTablePath (argv[2]);
    
    // Import the lookup table
    
    if (!exists(lookupTablePath) or !is_regular_file(lookupTablePath)) {
        std::cout << "Must provide good lookup table as second argument." << std::endl;
        return 1;
    }

    PadLookupTable lookupTable (lookupTablePath.string());
    
    // Find files in the provided directory
    
    std::vector<boost::filesystem::path> filePaths;
    
    try {
        filePaths = FindFilesInEventDir(rootDir);
    }
    catch (int) {
        std::cout << "An error occurred." << std::endl;     // FIX!
    }
    if (filePaths.size() == 0) {
        std::cout << "No files found in that directory." << std::endl;
        return 1;
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
    output.OpenFileForWrite("/Users/josh/Desktop/output.bin");
    
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
                                       [&] (const GETDataFile & x) {return x.eof();}),
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
    
    return 0;
}

