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
#include "Utilities.h"

class Event
{
public:
    
    // Construction of Events
    
    Event();
    Event(std::vector<uint8_t>& raw);
    Event(Event& orig);    // copy constructor
    Event(Event&& orig);   // move constructor
    
    Event& operator=(Event& orig);    // copy operator
    Event& operator=(Event&& orig);   // move operator
    
    // Setting properties
    
    void SetLookupTable(PadLookupTable* table);
    
    void SetEventId(uint32_t eventId_in);
    void SetEventTime(uint32_t eventTime_in);
    
    void AppendFrame(const GETFrame& frame);
    uint32_t Size() const;
    
    // Getting properties and members
    
    Trace& GetTrace(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel);
    
    uint32_t GetEventId() const;
    uint32_t GetEventTime() const;
    
    // Manipulations of contained data
    
    void SubtractFPN();
    
    // I/O functions
    
    friend std::ostream& operator<<(std::ostream& stream, const Event& event);
    
private:
    
    // Lookup table pointer and hash functions
    
    PadLookupTable *lookupTable;
    int CalculateHash(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel);
    
    // Event Header fields
    
    uint32_t eventId = 0;
    uint32_t eventTime = 0;
    static const uint8_t magic; // Equals 0xEE, defined in Event.cpp
    
    // Traces for each pad
    
    std::map<int,Trace> traces;
    
    friend class EventFile;
};

#endif /* defined(__GETConsolidate__Event__) */
