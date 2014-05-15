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
#include "GETExceptions.h"
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
    
    static const char* magic; // "EVT": 4 char, null-term
    
    // Traces for each pad
    std::map<int,Trace> traces;
    
    friend class EventFile;
    
    template<typename outType>
    outType ExtractInt(std::vector<char>::const_iterator begin,
                       std::vector<char>::const_iterator end);
    
public:
    Event();
    Event(std::vector<char>& raw);
    
    void SetLookupTable(PadLookupTable* table);
    void AppendFrame(const GETFrame& frame);
    uint32_t Size() const;
    
    Trace& GetTrace(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel);
    
    uint32_t GetEventId() const;
    uint32_t GetEventTime() const;
    
    void SetEventId(uint32_t eventId_in);
    void SetEventTime(uint32_t eventTime_in);
    
    void SubtractFPN();
    
    friend std::ostream& operator<<(std::ostream& stream, const Event& event);
    friend std::istream& operator>>(std::istream& stream, const Event& event);
};

#endif /* defined(__GETConsolidate__Event__) */
