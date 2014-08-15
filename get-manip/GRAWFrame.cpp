//
//  GRAWFrame.cpp
//  get-manip
//
//  Created by Joshua Bradt on 5/5/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "GRAWFrame.h"

// --------
// Static constants
// --------

const uint8_t  GRAWFrame::Expected_metaType = 8;
const uint16_t GRAWFrame::Expected_headerSize = 1;
const uint16_t GRAWFrame::Expected_itemSizePartialReadout = 4;
const uint16_t GRAWFrame::Expected_itemSizeFullReadout = 2;
const uint16_t GRAWFrame::Expected_frameTypePartialReadout = 1;
const uint16_t GRAWFrame::Expected_frameTypeFullReadout = 2;
const int      GRAWFrame::sizeUnit = 256;

// --------
// Constructor
// --------

GRAWFrame::GRAWFrame()
: metaType(6),frameSize(0),dataSource(0),frameType(1),revision(4),headerSize(2),itemSize(4),nItems(0),eventTime(0),eventId(0),coboId(0),asadId(0),readOffset(0),status(0)
{
    hitPatterns = {0,0,0,0};
    multiplicity = {0,0,0,0};
}

GRAWFrame::GRAWFrame(const std::vector<uint8_t>& rawFrame, const uint8_t fileCobo, const uint8_t fileAsad)
{
    auto rawFrameIter = rawFrame.begin();
    
//    std::cout << "Parsing raw frame." << std::endl;
    
    metaType = *rawFrameIter;
    rawFrameIter++;
    if (metaType != Expected_metaType) {
        std::cout << "    Unexpected metaType " << int(metaType) << std::endl;
    }
    
    frameSize = Utilities::ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter + 3);
    rawFrameIter += 3;
    if (frameSize*sizeUnit != rawFrame.size()) {
        std::cout << "    Wrong frameSize. Using raw frame size." << std::endl;
        frameSize = static_cast<decltype(frameSize)>(rawFrame.size()/sizeUnit);
    }
    
    dataSource = *rawFrameIter;
    rawFrameIter++;
    
    frameType = Utilities::ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter += 2;
    if (frameType != Expected_frameTypeFullReadout and
        frameType != Expected_frameTypePartialReadout) {
        std::cout << "    Unknown frameType. Read will likely fail." << std::endl;
    }
    
    revision = *rawFrameIter;
    rawFrameIter++;
    
    headerSize = Utilities::ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter += 2;
    if (headerSize != Expected_headerSize) {
        std::cout << "    Wrong headerSize " << int(headerSize) << ". Correcting." << std::endl;
        headerSize = Expected_headerSize;
    }
    
    itemSize = Utilities::ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter += 2;
    if ((frameType == Expected_frameTypePartialReadout and
         itemSize != Expected_itemSizePartialReadout) or
        (frameType == Expected_frameTypeFullReadout and
         itemSize != Expected_itemSizeFullReadout)) {
            std::cout << "    Wrong itemSize " << int(itemSize) << ". Correcting." << std::endl;
            if (frameType == Expected_frameTypePartialReadout) {
                itemSize = Expected_itemSizePartialReadout;
            }
            else if (frameType == Expected_frameTypeFullReadout) {
                itemSize = Expected_itemSizeFullReadout;
            }
    }
    
    nItems = Utilities::ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter+4);
    rawFrameIter += 4;
    if (frameSize != ceil(double(nItems*itemSize + headerSize*sizeUnit)/sizeUnit)) {
        std::cout << "    Mismatched nItems. Correcting." << std::endl;
        nItems = (frameSize*sizeUnit - headerSize*sizeUnit)/itemSize;
    }
    
    eventTime = Utilities::ExtractByteSwappedInt<uint64_t>(rawFrameIter, rawFrameIter+6);
    rawFrameIter += 6;
    
    assert((eventTime & 0xff00000000000000) == 0);
    
    eventId = Utilities::ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter+4);
    rawFrameIter += 4;
    
    coboId = *rawFrameIter;
    rawFrameIter++;
    
    if (coboId != fileCobo) {
        // This is the usual case.
        coboId = fileCobo;
    }
    
    asadId = *rawFrameIter;
    rawFrameIter++;
    
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
        for (int byte = 8; byte >=0; byte--) {
            std::bitset<72> temp {*rawFrameIter};
            temp <<= byte*8;
            bs |= temp;
            rawFrameIter++;
        }
        hitPatterns.push_back(bs);
    }
    
    for (int aget = 0; aget<4; aget++) {
        uint16_t mult_in = Utilities::ExtractByteSwappedInt<decltype(mult_in)>(rawFrameIter, rawFrameIter + sizeof(mult_in));
        rawFrameIter += sizeof(mult_in);
        multiplicity.push_back(mult_in);
    }
    
    // Extract data items
    auto dataBegin = rawFrame.begin() + headerSize*GRAWFrame::sizeUnit;
    auto dataEnd   = dataBegin + nItems*itemSize;
    
    if (frameType == Expected_frameTypePartialReadout) {
        ExtractPartialReadoutData(dataBegin, dataEnd);
    }
    else if (frameType == Expected_frameTypeFullReadout) {
        ExtractFullReadoutData(dataBegin, dataEnd);
    }
}

// --------
// Getters
// --------

const uint32_t GRAWFrame::GetEventId() const
{
    return eventId;
}

const uint64_t GRAWFrame::GetEventTime() const
{
    return eventTime;
}

// --------
// Data Extraction Functions
// --------

void GRAWFrame::ExtractPartialReadoutData(std::vector<uint8_t>::const_iterator& begin,
                                          std::vector<uint8_t>::const_iterator& end)
{
    std::vector< std::bitset<72> > actualHitPattern {0,0,0,0};
    
    for (auto rawFrameIter = begin; rawFrameIter != end; rawFrameIter+=itemSize) {
        uint32_t item = Utilities::ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter+itemSize);
        
        uint8_t aget    = ExtractAgetId(item);
        uint8_t channel = ExtractChannel(item);
        uint16_t tbid   = ExtractTBid(item);
        int16_t sample  = ExtractSample(item);
        
        assert(aget >= 0 and aget < 4);
        assert(channel >= 0 and channel < 68);
        assert(tbid >= 0 and tbid < 512);
        
        data.push_back(GRAWDataItem(aget,channel,tbid,sample));
        
        // WARNING: The hit pattern is in the reverse order of the bitset's accessor.
        
        actualHitPattern.at(aget).set(67-channel);
    }
    
    // Compare hit patterns
    
    unsigned int nMissing = 0;
    unsigned int nUnexpected = 0;
    
    for (int aget_iter = 0; aget_iter < 4; aget_iter++) {
        for (int ch_iter = 0; ch_iter < 68; ch_iter ++) {
            bool isExpected = hitPatterns.at(aget_iter).test(ch_iter);
            bool isFound = actualHitPattern.at(aget_iter).test(ch_iter);
            
            if (isExpected and !isFound) {
                nMissing++;
            }
            else if (isFound and !isExpected) {
                nUnexpected++;
            }
        }
    }
    
//    if (nMissing > 0)
//        LOG_WARNING << "Missing " << nMissing << " channels." << std::endl;
//    if (nUnexpected > 0)
//        LOG_WARNING << "Found " << nUnexpected << " unexpected channels." << std::endl;
//    
    if (data.size() != nItems) {
        LOG_WARNING << "Missing data items." << std::endl;
    }

}

void GRAWFrame::ExtractFullReadoutData(std::vector<uint8_t>::const_iterator& begin,
                                       std::vector<uint8_t>::const_iterator& end)
{
    std::vector<std::queue<int16_t>> dataQueues (4);
    
    for (auto rawFrameIter = begin; rawFrameIter != end; rawFrameIter += itemSize) {
        uint16_t item = Utilities::ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+itemSize);
        
        uint8_t aget    = ExtractAgetIdFullReadout(item);
        int16_t sample  = ExtractSampleFullReadout(item);
        
        assert(aget >= 0 and aget < 4);
        
        dataQueues.at(aget).push(sample);
    }
    
    for (uint8_t aget = 0; aget < 4; aget++) {
        for (uint16_t tbid = 0; tbid < 512; tbid ++) {
            for (uint8_t channel = 0; channel < 68; channel++) {
                auto sample = dataQueues.at(aget).front();
                dataQueues.at(aget).pop();
                
                data.push_back(GRAWDataItem(aget,channel,tbid,sample));
            }
        }
    }
}

uint8_t GRAWFrame::ExtractAgetId(const uint32_t raw)
{
    return (raw & 0xC0000000)>>30;
}

uint8_t GRAWFrame::ExtractChannel(const uint32_t raw)
{
    return (raw & 0x3F800000)>>23;
}

uint16_t GRAWFrame::ExtractTBid(const uint32_t raw)
{
    return (raw & 0x007FC000)>>14;
}

int16_t GRAWFrame::ExtractSample(const uint32_t raw)
{
    return (raw & 0x00000FFF);
}

uint8_t GRAWFrame::ExtractAgetIdFullReadout(const uint16_t raw)
{
    return (raw & 0xC000)>>14;
}

int16_t GRAWFrame::ExtractSampleFullReadout(const uint16_t raw)
{
    return (raw & 0x0FFF);
}

