//
//  GRAWFrame.h
//  get-manip
//
//  Created by Joshua Bradt on 5/5/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __get_manip__GRAWFrame__
#define __get_manip__GRAWFrame__

#include <iostream>
#include <vector>
#include <queue>
#include <bitset>
#include <cmath>
#include <assert.h>

#include "GRAWDataItem.h"
#include "Utilities.h"
#include "UserInterface.h"
#include "Constants.h"

class GRAWFrame
{
public:
    GRAWFrame();
    GRAWFrame(const std::vector<uint8_t>& rawFrame);
    
    // Getters
    
    const evtid_t GetEventId() const;
    const ts_t GetEventTime() const;
    
    // Data extraction functions
    
    void ExtractPartialReadoutData(std::vector<uint8_t>::const_iterator& begin,
                                   std::vector<uint8_t>::const_iterator& end);
    void ExtractFullReadoutData(std::vector<uint8_t>::const_iterator& begin,
                                std::vector<uint8_t>::const_iterator& end);
    
    static addr_t ExtractAgetId(const uint32_t raw);
    static addr_t ExtractChannel(const uint32_t raw);
    static tb_t ExtractTBid(const uint32_t raw);
    static sample_t ExtractSample(const uint32_t raw);
    static addr_t ExtractAgetIdFullReadout(const uint16_t raw);
    static sample_t ExtractSampleFullReadout(const uint16_t raw);
    
    static const uint8_t  Expected_metaType;
    static const uint16_t Expected_headerSize;
    static const uint16_t Expected_itemSizePartialReadout;
    static const uint16_t Expected_itemSizeFullReadout;
    static const uint16_t Expected_frameTypePartialReadout;
    static const uint16_t Expected_frameTypeFullReadout;
    static const int      sizeUnit;
    
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
    uint64_t eventTime;
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
    
    std::vector<GRAWDataItem> data;

    // Friends
    
    friend class Event;
    friend class GRAWFrameTestFixture;
    friend class GRAWFile;
};

#endif /* defined(__getmanip__GRAWFrame__) */
