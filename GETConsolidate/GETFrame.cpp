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

GETFrame::GETFrame()
: metaType(6),frameSize(0),dataSource(0),frameType(1),revision(4),headerSize(2),itemSize(4),nItems(0),eventTime(0),eventId(0),coboId(0),asadId(0),readOffset(0),status(0)
{
    hitPatterns = {0,0,0,0};
    multiplicity = {0,0,0,0};
}

GETFrame::GETFrame(const std::vector<uint8_t>& rawFrame, const uint8_t fileCobo, const uint8_t fileAsad)
{
    auto rawFrameIter = rawFrame.begin();
    
//    std::cout << "Parsing raw frame." << std::endl;
    
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
    
    eventTime = Utilities::ExtractByteSwappedInt<uint64_t>(rawFrameIter, rawFrameIter+6);
    rawFrameIter += 6;
    
    eventId = Utilities::ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter+4);
    rawFrameIter += 4;
    
    coboId = *rawFrameIter;
    rawFrameIter++;
    
    if (coboId != fileCobo) {
//        std::cout << "    CoBo ID in file does not match CoBo ID in path. Using path value." << std::endl;
        coboId = fileCobo;
    }
    
    asadId = *rawFrameIter;
    rawFrameIter++;
    
//    std::cout << "    This frame is for CoBo " << (int) coboId << " AsAd " << (int) asadId << std::endl;
    
    if (asadId != fileAsad) {
        std::cout << "    AsAd ID in file does not match AsAd ID in path. Using path value." << std::endl;
        asadId = fileAsad;
    }
    
    readOffset = Utilities::ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter+=2;
    
    status = *rawFrameIter;
    rawFrameIter++;
    
    for (int aget = 0; aget<4; aget++) {
        std::bitset<9*8> bs {};   // init to 0
        for (int byte = 8; byte>=0; byte--) {
            //hitPattern[aget][byte] = *rawFrameIter;
            bs &= (*rawFrameIter) << byte*8;
//            std::cout << int(*rawFrameIter) << std::endl;
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
        
        uint8_t aget    = ExtractAgetId(item);
        uint8_t channel = ExtractChannel(item);
        uint16_t tbid   = ExtractTBid(item);
        int16_t sample  = ExtractSample(item);
        
        data.push_back(GETFrameDataItem(aget,channel,tbid,sample));
        
//        if (!(hitPatterns.at(aget).test(channel))) {
//            std::cout << "Channel " << int(channel) << " on CoBo " << int(coboId) << " AsAd " << int(asadId) << " AGET " << int(aget) << " not on in hitpattern." << std::endl;
//        }
    }
    
    if (data.size() != nItems) {
        std::cout << "Missing data items." << std::endl;
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

