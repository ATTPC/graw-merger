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

#include "GETFrameDataItem.h"

class GETFrameDataItem;

class GETFrame
{
    // Friends
    friend class Event;

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
    uint8_t hitPattern[4][9];
    uint16_t multiplicity[4];
    
    // Data items
    std::vector<GETFrameDataItem*> *data;
    
public:
    GETFrame(std::vector<uint8_t> *rawFrame, uint8_t file_cobo, uint8_t file_asad);
    ~GETFrame();
    
    template<typename outType>
    static outType ExtractByteSwappedInt(std::vector<uint8_t>::iterator begin,std::vector<uint8_t>::iterator end);
};

#endif /* defined(__GETConsolidate__GETFrame__) */
