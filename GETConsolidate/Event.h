//
//  Event.h
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __GETConsolidate__Event__
#define __GETConsolidate__Event__

#include <iostream>
#include <vector>
#include "GETFrame.h"
#include "GETFrameDataItem.h"
#include "Trace.h"
#include "PadLookupTable.h"
#include <map>

class Event
{
private:
    // Lookup table pointer and hash functions
    PadLookupTable *lookupTable;
    int CalculateHash(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel);
    
    // Event Header fields
    
    uint32_t eventId = 0;
    uint32_t eventTime = 0;
    
    // Traces for each pad
    std::map<int,Trace*> *traces;
    
    
public:
    Event();
    ~Event();
    
    void SetLookupTable(PadLookupTable* table);
    void AppendFrame(GETFrame* frame);
    
    friend std::ostream& operator<<(std::ostream& stream, const Event& event);
};

#endif /* defined(__GETConsolidate__Event__) */
