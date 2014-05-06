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

std::vector<boost::filesystem::path*>* findFilesInEventDir(boost::filesystem::path eventRoot)
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

int main(int argc, const char * argv[])
{
    std::ifstream testFile;
    std::vector<uint8_t> *frame_raw;
    
    boost::filesystem::path rootDir (argv[1]);
    
    std::vector<boost::filesystem::path*> *files = findFilesInEventDir(rootDir);
    
    for (auto filename : *files) {
        std::cout << (*filename).string() << std::endl;
    }
    
//    testFile.open("/Users/josh/Dropbox/Research/sample.graw",std::ios::in|std::ios::binary);
//    if (testFile.good()) {
//        std::vector<uint8_t> size_raw;
//        uint16_t size;
//        
//        testFile.seekg(1,std::ios::cur);
//        
//        for (int i = 0; i < 3; i++) {
//            char temp;
//            testFile.read(&temp, sizeof(uint8_t));
//            size_raw.push_back((uint8_t)temp);
//        }
//        
//        size = GETFrame::ExtractByteSwappedInt<uint32_t>(size_raw.begin(), size_raw.end());
//        std::cout << "Found frame of size " << size << std::endl;
//        
//        testFile.seekg(-4,std::ios::cur); // rewind to start of frame
//        
//        frame_raw = new std::vector<uint8_t>;
//        
//        for (unsigned long i = 0; i < size*64; i++) {
//            char temp;
//            testFile.read(&temp, sizeof(uint8_t));
//            frame_raw->push_back((uint8_t)temp);
//        }
//        
//        std::cout << "Read " << size*64 << " bytes from file." << std::endl;
//        
//        GETFrame *frame = new GETFrame(frame_raw);
//        
//        testFile.close();
//        
//        std::cout << "Done" << std::endl;
//        delete frame;
//        delete frame_raw;
//    }
//    else {
//        std::cout << "Bad file." << std::endl;
//    }
    
    return 0;
}

