//
//  Event.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "Event.h"

Event::Event()
{
    traces = new std::map<int,Trace*>;
}

int Event::CalculateHash(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel)
{
    return channel + aget*100 + asad*10000 + cobo*1000000;
}

void Event::SetLookupTable(PadLookupTable * table)
{
    lookupTable = table;
}

void Event::AppendFrame(GETFrame *frame)
{
    // Make sure pointers to required objects are valid
    if (frame == NULL) {
        std::cout << "Error: Invalid frame passed to Event." << std::endl;
        return;
    }
    if (lookupTable == NULL) {
        std::cout << "Error: No lookup table provided to Event." << std::endl;
    }
    
    // Get header information from frame
    
    auto cobo = frame->coboId;
    auto asad = frame->asadId;
    
    // Extract data items and create traces for them
    
    for (GETFrameDataItem* dataItem : *(frame->data)) {
        // Extract information
        auto aget = dataItem->GetAgetId();
        auto channel = dataItem->GetChannel();
        auto tbid = dataItem->GetTimeBucketId();
        auto sample = dataItem->GetSample();
        
        int hash = CalculateHash(cobo, asad, aget, channel);
        auto pad = lookupTable->FindPadNumber(cobo, asad, aget, channel);
        
        // Find trace in hash table, if it exists
        auto foundTrace = traces->find(hash);
        if (foundTrace == traces->end()) {
            // The trace doesn't exist, so create it and put it in the table.
            Trace* newTrace = new Trace(cobo, asad, aget, channel, pad);
            newTrace->AppendSample(tbid, sample);
            traces->emplace(hash,newTrace);
        }
        else {
            // The trace already exists. Append this sample to it.
            foundTrace->second->AppendSample(tbid, sample);
        }
    }
}

Event::~Event()
{
    for (auto item : *traces) {
        delete item.second;
    }
    delete traces;
    
    // Event class should not own the lookup table, so it shouldn't have to
    // delete the table when it is destroyed.
}

std::ostream& operator<<(std::ostream& stream, const Event& event)
{
    for (auto item : *(event.traces))
    {
        stream << *(item.second);
    }

    return stream;
}