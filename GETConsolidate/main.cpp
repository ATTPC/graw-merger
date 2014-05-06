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
                    std::cout << "Found file: " << (*dirIter).path().filename().string() << std::endl;
                    boost::filesystem::path *file = new boost::filesystem::path (dirIter->path());
                    filesFound->push_back(file);
                }
            }
        }
    }
    catch (const boost::filesystem::filesystem_error& ex) {
                std::cout << ex.what() << std::endl;
    }
    
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
    std::cout << "Found frame of size " << size << std::endl;
    
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
    
    std::vector<std::ifstream*> fileStreams;

    // Open all of the files
    
    for (auto filename : *files) {
        std::ifstream *filePtr = new std::ifstream(filename->string(),std::ios::in|std::ios::binary);
        if (filePtr->good()) {
            fileStreams.push_back(filePtr);
        }
        else {
            std::cout << "Failed to open file: " << filename->filename().string() << std::endl;
        }
    }
    
    // Extract first frame from each
    
    std::vector<GETFrame*> *frames = new std::vector<GETFrame*>;
    
    for (auto file : fileStreams) {
        GETFrame *frame = new GETFrame(GetRawFrame(file));
        frames->push_back(frame);
    }
    
    
    
    return 0;
}

