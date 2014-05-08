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
#include <exception>
#include <queue>

long int gTotalSize = 0;

class FrameReadFail : public std::exception
{
public:
    virtual const char* what()
    {
        return "Failed to read frame from file.";
    }
} frameReadFail;

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

std::vector<uint8_t>* GetRawFrame(std::ifstream* file)
{
    std::vector<uint8_t> size_raw;
    uint16_t size;
    std::vector<uint8_t> *frame_raw;
    
    file->seekg(1,std::ios::cur);
    
    for (int i = 0; i < 3; i++) {
        char temp;
        file->read(&temp, sizeof(uint8_t));
        size_raw.push_back((uint8_t)temp);
    }
    
    size = GETFrame::ExtractByteSwappedInt<uint32_t>(size_raw.begin(), size_raw.end());
    
    if (size == 0) throw frameReadFail;
    
    std::cout << "Found frame of size " << size << std::endl;
    gTotalSize += size;
    
    file->seekg(-4,std::ios::cur); // rewind to start of frame
    
    frame_raw = new std::vector<uint8_t>;
    
    for (unsigned long i = 0; i < size*64; i++) {
        char temp;
        file->read(&temp, sizeof(uint8_t));
        frame_raw->push_back((uint8_t)temp);
    }
    
    return frame_raw;
    // Leaves file pointer at end of first frame. This assumes the frame size is correct.
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
    
    std::vector<boost::filesystem::path*> *files = FindFilesInEventDir(rootDir);
    if (files == NULL) {
        std::cout << "No files found in that directory. Does it exist?" << std::endl;
        return 1;
    }
    
    std::queue<std::ifstream*> fileStreams_inbox;
    std::queue<std::ifstream*> fileStreams_outbox;

    // Open all of the files
    
    for (auto filename : *files) {
        std::ifstream *filePtr = new std::ifstream(filename->string(),std::ios::in|std::ios::binary);
        if (filePtr->good()) {
            fileStreams_inbox.push(filePtr);
        }
        else {
            std::cout << "Failed to open file: " << filename->filename().string() << std::endl;
        }
    }
    
    std::ofstream output ("/Users/josh/Desktop/output.bin", std::ios::out|std::ios::binary);
    
    // Main loop
    
    while (!fileStreams_inbox.empty() and fileStreams_outbox.empty()) {
       
        std::vector<GETFrame*> *frames = new std::vector<GETFrame*>;
        
        // Read in a frame from each file
        
        while (!fileStreams_inbox.empty()) {
            // Create a frame.
            std::ifstream* currentFileStream = fileStreams_inbox.front();
            fileStreams_inbox.pop();
            
//            try {
//                GETFrame *frame = new GETFrame(GetRawFrame(currentFileStream));
//                frames->push_back(frame);
//                fileStreams_outbox.push(currentFileStream);
//            }
//            catch (FrameReadFail& fail) {
//                std::cout << fail.what() << std::endl;
//                currentFileStream->close();
//                delete currentFileStream;
//            }
            try {
                GETFrame *frame = new GETFrame(GetRawFrame(currentFileStream));
                frames->push_back(frame);
                fileStreams_outbox.push(currentFileStream);
            }
            catch (FrameReadFail& fail) {
                std::cout << fail.what() << std::endl;
                currentFileStream->close();
                delete currentFileStream;
            }
        }
        
        // Put the filestreams back in the input queue
        std::swap(fileStreams_outbox, fileStreams_inbox);
        
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
    
    for (auto item : *files) {
        delete item;
    }
    delete files;
    
    return 0;
}

