//
//  main.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/5/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "GETFrame.h"



int main(int argc, const char * argv[])
{
    std::ifstream testFile;
    std::vector<uint8_t> *frame_raw;
    
    testFile.open("/Users/josh/Desktop/sample.graw",std::ios::in|std::ios::binary);
    if (testFile.good()) {
        std::vector<uint8_t> size_raw;
        uint16_t size;
        
        testFile.seekg(1,std::ios::cur);
        
        for (int i = 0; i < 3; i++) {
            char temp;
            testFile.read(&temp, sizeof(uint8_t));
            size_raw.push_back((uint8_t)temp);
        }
        
        size = GETFrame::ExtractByteSwapInt16(size_raw.begin(), size_raw.end());
        std::cout << "Found frame of size " << size << std::endl;
        
        testFile.seekg(-4,std::ios::cur); // rewind to start of frame
        
        frame_raw = new std::vector<uint8_t>;
        
        for (unsigned long i = 0; i < size*64; i++) {
            char temp;
            testFile.read(&temp, sizeof(uint8_t));
            frame_raw->push_back((uint8_t)temp);
        }
        
        std::cout << "Read " << size*64 << " bytes from file." << std::endl;
        
        GETFrame *frame = new GETFrame(frame_raw);
        
        testFile.close();
    }
    else {
        std::cout << "Bad file." << std::endl;
    }
    
    return 0;
}

