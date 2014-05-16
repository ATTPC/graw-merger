//
//  GETFrame.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/5/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "GETFrame.h"

// --------
// Constructor
// --------

GETFrame::GETFrame(GETDataFile& file)
{
    auto rawFrame = file.GetNextRawFrame();
    auto rawFrameIter = rawFrame.begin();
    
    std::cout << "Parsing raw frame." << std::endl;
    
    metaType = *rawFrameIter;
    rawFrameIter++;
    if (metaType != 6) {
        std::cout << "    Wrong metaType " << metaType << ", should be 6." << std::endl;
    }
    
    frameSize = Utilities::ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter + 3);
    rawFrameIter += 3;
    
    dataSource = *rawFrameIter;
    rawFrameIter++;
    
    frameType = Utilities::ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter += 2;
    
    revision = *rawFrameIter;
    rawFrameIter++;
    
    headerSize = Utilities::ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter += 2;
    if (headerSize != 4) {
        std::cout << "    Wrong headerSize " << headerSize << ", should be 4. Correcting." << std::endl;
        headerSize = 4;
    }
    
    itemSize = Utilities::ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter += 2;
    if (itemSize != 4) {
        std::cout << "    Wrong itemSize " << itemSize << ", should be 4. Correcting." << std::endl;
        itemSize = 4;
    }
    
    nItems = Utilities::ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter+4);
    rawFrameIter += 4;
    if (frameSize*64 != nItems*itemSize + headerSize*64) {
        std::cout << "    Mismatched frameSize. Correcting nItems." << std::endl;
        nItems = (frameSize*64 - headerSize*64)/4;
    }
    
    eventTime = Utilities::ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter+6);
    rawFrameIter += 6;
    
    eventId = Utilities::ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter+4);
    rawFrameIter += 4;
    
    coboId = *rawFrameIter;
    rawFrameIter++;
    
    if (coboId != file.GetFileCobo()) {
        std::cout << "    CoBo ID in file does not match CoBo ID in path. Using path value." << std::endl;
        coboId = file.GetFileCobo();
    }
    
    asadId = *rawFrameIter;
    rawFrameIter++;
    
    std::cout << "    This frame is for CoBo " << (int) coboId << " AsAd " << (int) asadId << std::endl;
    
    if (asadId != file.GetFileAsad()) {
        std::cout << "    AsAd ID in file does not match AsAd ID in path. Using path value." << std::endl;
        asadId = file.GetFileAsad();
    }
    
    readOffset = Utilities::ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter+2;
    
    status = *rawFrameIter;
    rawFrameIter++;
    
    for (int aget = 0; aget<4; aget++) {
        std::bitset<9*8> bs {};   // init to 0
        for (int byte = 0; byte<9; byte++) {
            //hitPattern[aget][byte] = *rawFrameIter;
            bs &= (*rawFrameIter) << byte*8;
            rawFrameIter++;
        }
        hitPatterns.push_back(bs);
    }
    
    for (int aget = 0; aget<4; aget++) {
        multiplicity.push_back(*rawFrameIter);
        rawFrameIter++;
    }
    
    // Extract data items
    auto dataBegin = rawFrame.begin() + headerSize*64;
    
    for (rawFrameIter = dataBegin; rawFrameIter != rawFrame.end(); rawFrameIter+=4) {
        uint32_t item = Utilities::ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter+4);
        //        std::cout << std::hex << item << std::dec << std::endl;
        
        uint8_t aget    = ExtractAgetId(item);
        uint8_t channel = ExtractChannel(item);
        uint16_t tbid   = ExtractTBid(item);
        int16_t sample  = ExtractSample(item);
        
        data.push_back(GETFrameDataItem(aget,channel,tbid,sample));
    }
}

// --------
// Private Data Extraction Functions
// --------

uint8_t GETFrame::ExtractAgetId(const uint32_t raw)
{
    return (raw & 0xC0000000)>>30;
}

uint8_t GETFrame::ExtractChannel(const uint32_t raw)
{
    return (raw & 0x3F800000)>>23;
}

uint16_t GETFrame::ExtractTBid(const uint32_t raw)
{
    return (raw & 0x007FC000)>>14;
}

int16_t GETFrame::ExtractSample(const uint32_t raw)
{
    return (raw & 0x00000FFF);
}

