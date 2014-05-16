//
//  GETFrame.h
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/5/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __GETConsolidate__GETFrame__
#define __GETConsolidate__GETFrame__

#include <iostream>
#include <vector>
#include <bitset>

#include "GETFrameDataItem.h"
#include "GETDataFile.h"
#include "Utilities.h"

class GETFrame
{
    // Friends
    friend class Event;
    friend class GETFrameTestFixture;

private:
    // Header fields
    uint8_t metaType; // set to 0x6
    uint32_t frameSize; // in units of 64 bytes
    uint8_t dataSource;
    uint16_t frameType;
    uint8_t revision;
    uint16_t headerSize;
    uint16_t itemSize;
    uint32_t nItems;
    uint32_t eventTime;
    uint32_t eventId;
    uint8_t coboId;
    uint8_t asadId;
    uint16_t readOffset;
    uint8_t status;
//    uint8_t hitPattern[4][9];
    std::vector< std::bitset<9*8> > hitPatterns;
//    uint16_t multiplicity[4];
    std::vector<uint8_t> multiplicity;
    
    // Data items
    std::vector<GETFrameDataItem> data;
    
public:
    GETFrame(GETDataFile& file);
    
    static uint8_t ExtractAgetId(const uint32_t raw);
    static uint8_t ExtractChannel(const uint32_t raw);
    static uint16_t ExtractTBid(const uint32_t raw);
    static int16_t ExtractSample(const uint32_t raw);
};

#endif /* defined(__GETConsolidate__GETFrame__) */
