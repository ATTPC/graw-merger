//
//  GETFrame.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/5/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "GETFrame.h"

template<typename outType>
outType GETFrame::ExtractByteSwappedInt(std::vector<uint8_t>::iterator begin,
                              std::vector<uint8_t>::iterator end)
{
    outType result = 0;
    int n = 0;
    for (auto iter = end-1; iter >= begin; iter--) {
        result |= (*iter)<<(8*n);
        n++;
    }
    return result;
}

GETFrame::GETFrame(std::vector<uint8_t> *rawFrame)
{
    auto rawFrameIter = rawFrame->begin();
    
    metaType = *rawFrameIter;
    rawFrameIter++;
    if (metaType != 6) {
        std::cout << "Wrong metaType " << metaType << ", should be 6." << std::endl;
    }
    
    frameSize = ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter + 3);
    rawFrameIter += 3;
    
    dataSource = *rawFrameIter;
    rawFrameIter++;
    
    frameType = ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter += 2;
    
    revision = *rawFrameIter;
    rawFrameIter++;
    
    headerSize = ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter += 2;
    if (headerSize != 4) {
        std::cout << "Wrong headerSize " << headerSize << ", should be 4. Correcting." << std::endl;
        headerSize = 4;
    }
    
    itemSize = ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter += 2;
    if (itemSize != 4) {
        std::cout << "Wrong itemSize " << itemSize << ", should be 4. Correcting." << std::endl;
        itemSize = 4;
    }
    
    nItems = ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter+4);
    rawFrameIter += 4;
    if (frameSize*64 != nItems*itemSize + headerSize*64) {
        std::cout << "Mismatched frameSize. Correcting nItems." << std::endl;
        nItems = (frameSize*64 - headerSize*64)/4;
    }
    
    eventTime = ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter+6);
    rawFrameIter += 6;
    
    eventId = ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter+4);
    rawFrameIter += 4;
    
    coboId = *rawFrameIter;
    rawFrameIter++;
    
    asadId = *rawFrameIter;
    rawFrameIter++;
    
    readOffset = ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter+2;
    
    status = *rawFrameIter;
    rawFrameIter++;
    
    for (int aget = 0; aget<4; aget++) {
        for (int byte = 0; byte<9; byte++) {
            hitPattern[aget][byte] = *rawFrameIter;
            rawFrameIter++;
        }
    }
    
    for (int aget = 0; aget<4; aget++) {
        multiplicity[aget] = *rawFrameIter;
        rawFrameIter++;
    }
    
    // Extract data items
    auto dataBegin = rawFrame->begin() + headerSize*64;

    data = new std::vector<GETFrameDataItem*>;
    
    for (rawFrameIter = dataBegin; rawFrameIter != rawFrame->end(); rawFrameIter+=4) {
        uint32_t item = ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter+4);
//        std::cout << std::hex << item << std::dec << std::endl;
        
        uint8_t aget    =         (item & 0xC0000000)>>30;
        uint8_t channel =         (item & 0x3F800000)>>22;
        uint16_t tbid   =         (item & 0x007FC000)>>14;
        float sample    = (float) (item & 0x00000FFF);
        
        GETFrameDataItem* dataItem = new GETFrameDataItem(aget,channel,tbid,sample);
        data->push_back(dataItem);
        dataItem = NULL;
    }
}

GETFrame::~GETFrame()
{
    for (auto item : *data) {
        delete item;
    }
    delete data;
}