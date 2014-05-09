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
    
    uint8_t cobo = frame->coboId;
    uint8_t asad = frame->asadId;
    
    if (this->eventId == 0) {
        this->eventId = frame->eventId;
    }
    else if (this->eventId != frame->eventId) {
        std::cout << "Appended frame's event ID doesn't match. CoBo " << (int) cobo << ", AsAd " << (int) asad << std::endl;
    }
    
    if (this->eventTime == 0) {
        this->eventTime = frame->eventTime;
    }
    else if (this->eventTime != frame->eventTime) {
        std::cout << "Appended frame's event time doesn't match. CoBo " << (int) cobo << ", AsAd " << (int) asad << std::endl;
    }
    
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

uint32_t Event::Size() const
{
    // Size depends on what is written to disk. This is defined by
    // the stream insertion operator.
    
    uint32_t size = sizeof("EVT") + sizeof(uint32_t) + sizeof(eventId) + sizeof(eventTime) + sizeof(uint16_t);
    for (auto item : *traces) {
        size += item.second->Size();
    }
    return size;
}

std::ostream& operator<<(std::ostream& stream, const Event& event)
{
    uint32_t sizeOfEvent = event.Size();
    
    stream.write("EVT", sizeof("EVT"));
    
    stream.write((char*) &sizeOfEvent, sizeof(sizeOfEvent));
    stream.write((char*) &event.eventId, sizeof(event.eventId));
    stream.write((char*) &event.eventTime, sizeof(event.eventTime));
    
    uint16_t nTraces = (uint16_t) event.traces->size();
    stream.write((char*) &nTraces, sizeof(nTraces));
    
    for (auto item : *(event.traces))
    {
        stream << *(item.second);
    }

    return stream;
}