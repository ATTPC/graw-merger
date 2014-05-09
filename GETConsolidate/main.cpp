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
#include <exception>
#include <queue>

std::vector<boost::filesystem::path*>* FindFilesInEventDir(boost::filesystem::path eventRoot)
{
    std::vector<boost::filesystem::path*> *filesFound = NULL;
    
    try {
        if (exists(eventRoot)) {
            boost::filesystem::recursive_directory_iterator dirIter(eventRoot);
            boost::filesystem::recursive_directory_iterator endOfDir;
            filesFound = new std::vector<boost::filesystem::path*>;
            
            for ( ; dirIter != endOfDir; dirIter++) {
                if (is_directory(dirIter->path())) {
                    std::cout << "Entering directory: " << (*dirIter).path().string() << std::endl;
                }
                else if (is_regular_file(dirIter->path()) && (*dirIter).path().extension() == ".graw") {
                    std::cout << "    Found file: " << (*dirIter).path().filename().string() << std::endl;
                    boost::filesystem::path *file = new boost::filesystem::path (dirIter->path());
                    filesFound->push_back(file);
                }
            }
        }
        else {
            std::cout << "Directory does not exist." << std::endl;
            return NULL;
        }
    }
    catch (const boost::filesystem::filesystem_error& ex) {
                std::cout << ex.what() << std::endl;
    }
    
    std::cout << "Found " << filesFound->size() << " GRAW files." << std::endl;
    
    return filesFound;
}

int main(int argc, const char * argv[])
{
    boost::filesystem::path rootDir (argv[1]); // check these
    boost::filesystem::path lookupTablePath (argv[2]);
    
    // Import the lookup table
    
    PadLookupTable* lookupTable;
    
    std::ifstream lookupTableStream (lookupTablePath.string(), std::ios::in);
    if (lookupTableStream.good()) {
        lookupTable = new PadLookupTable(lookupTableStream);
    }
    else {
        std::cout << "Must provide good lookup table as second argument." << std::endl;
        return 1;
    }
    lookupTableStream.close();
    
    // Find files in the provided directory
    
    std::vector<boost::filesystem::path*> *files = FindFilesInEventDir(rootDir);
    if (files == NULL) {
        std::cout << "No files found in that directory. Does it exist?" << std::endl;
        return 1;
    }
    
    std::queue<GETDataFile*> dataFiles_inbox;
    std::queue<GETDataFile*> dataFiles_outbox;

    // Open all of the files
    
    for (auto filename : *files) {
        try {
            GETDataFile* newFile = new GETDataFile (filename->string());
            dataFiles_inbox.push(newFile);
        }
        catch (std::exception& e) {
            std::cout << "Exception thrown when opening file " << filename->string() << ": " << e.what() << std::endl;
        }
    }
    
    // Delete the paths since we're done with them
    
    for (auto item : *files) {
        delete item;
    }
    delete files;
    
    // Open the output file
    
    std::ofstream output ("/Users/josh/Desktop/output.bin", std::ios::out|std::ios::binary);
    
    // Main loop
    
    while (!dataFiles_inbox.empty() or !dataFiles_outbox.empty()) {
       
        std::vector<GETFrame*> *frames = new std::vector<GETFrame*>;
        
        // Read in a frame from each file
        
        while (!dataFiles_inbox.empty()) {
            // Create a frame.
            GETDataFile* currentFile = dataFiles_inbox.front();
            dataFiles_inbox.pop();
            try {
                GETFrame *frame = new GETFrame(currentFile->GetNextRawFrame());
                frames->push_back(frame);
                dataFiles_outbox.push(currentFile);
            }
            catch (std::exception& fail) {
                std::cout << fail.what() << std::endl;
                delete currentFile;
            }
        }
        
        // Put the data files back in the input queue
        std::swap(dataFiles_outbox, dataFiles_inbox);
        
        // Create an event from this set of frames, if there are any.
        
        if (frames->empty()) break;
        
        Event* testEvent = new Event();
        testEvent->SetLookupTable(lookupTable);
        
        for (auto frame : *frames) {
            testEvent->AppendFrame(frame);
            std::cout << "Appended frame." << std::endl;
        }
        
        // Now we're done with these frames, so delete them.
        
        for (auto item : *frames) {
            delete item;
        }
        delete frames;
        
        // Write the event to the output file.

        if (output.good())
        {
            output << *testEvent << std::flush;
        }
        else {
            std::cout << "Bad output file." << std::endl;
        }
        
        // Now throw out the event.
        
        delete testEvent;
    }
    
    
    delete lookupTable;
    

    
    return 0;
}

