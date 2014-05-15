//
//  Event.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "Event.h"

const char* Event::magic = "EVT";

template<typename outType>
outType Event::ExtractInt(std::vector<char>::const_iterator begin,
                          std::vector<char>::const_iterator end)
{
    outType result = 0;
    int n = 0;
    for (auto iter = begin; iter != end; iter++) {
        result |= (*iter)<<(8*n);
        n++;
    }
    return result;
}

Event::Event()
{
}

Event::Event(std::vector<char>& raw)
{
    char magic_in[4] {};
    auto rawIter = raw.begin();
    
    for (int i = 0; i < 4; i++) {
        magic_in[i] = *rawIter;
        rawIter++;
    }
    if (magic_in != Event::magic) throw Exceptions::Wrong_File_Position();
    
//    uint32_t sizeOfEvent = ExtractInt<uint32_t>(rawIter, rawIter+4);
    rawIter += 4;
    
    eventId = ExtractInt<decltype(eventId)>(rawIter, rawIter+sizeof(eventId));
    rawIter += sizeof(eventId);
    
    eventTime = ExtractInt<decltype(eventTime)>(rawIter, rawIter+sizeof(eventTime));
    rawIter += sizeof(eventTime);
    
    uint16_t nTraces = ExtractInt<uint16_t>(rawIter, rawIter+sizeof(uint16_t));
    rawIter += sizeof(uint16_t);
    
    for (decltype(nTraces) n = 0; n < nTraces; n++) {
        // Find trace size
        uint32_t traceSize = ExtractInt<decltype(traceSize)>(rawIter, rawIter+sizeof(traceSize));
        std::vector<char> rawTrace {rawIter,rawIter+traceSize};
        
        // Create and emplace the new trace
        Trace newTrace {rawTrace};
        auto newHash = CalculateHash(newTrace.coboId, newTrace.asadId, newTrace.agetId, newTrace.channel);
        traces.emplace(newHash,newTrace);
        
        // Increment the iterator
        rawIter += traceSize;
    }
}

int Event::CalculateHash(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel)
{
    return channel + aget*100 + asad*10000 + cobo*1000000;
}

void Event::SetLookupTable(PadLookupTable * table)
{
    lookupTable = table;
}

void Event::AppendFrame(const GETFrame& frame)
{
    // Make sure pointers to required objects are valid

    if (lookupTable == NULL) {
        std::cout << "Error: No lookup table provided to Event." << std::endl;
    }
    
    // Get header information from frame
    
    uint8_t cobo = frame.coboId;
    uint8_t asad = frame.asadId;
    
    if (this->eventId == 0) {
        this->eventId = frame.eventId;
    }
    else if (this->eventId != frame.eventId) {
        std::cout << "Appended frame's event ID doesn't match. CoBo " << (int) cobo << ", AsAd " << (int) asad << std::endl;
    }
    
    if (this->eventTime == 0) {
        this->eventTime = frame.eventTime;
    }
    else if (this->eventTime != frame.eventTime) {
        std::cout << "Appended frame's event time doesn't match. CoBo " << (int) cobo << ", AsAd " << (int) asad << std::endl;
    }
    
    // Extract data items and create traces for them
    
    for (auto dataItem : frame.data) {
        // Extract information
        auto aget = dataItem.GetAgetId();
        auto channel = dataItem.GetChannel();
        auto tbid = dataItem.GetTimeBucketId();
        auto sample = dataItem.GetSample();
        
        int hash = CalculateHash(cobo, asad, aget, channel);
        auto pad = lookupTable->FindPadNumber(cobo, asad, aget, channel);
        
        // Find trace in hash table, if it exists
        auto foundTrace = traces.find(hash);
        if (foundTrace == traces.end()) {
            // The trace doesn't exist, so create it and put it in the table.
            Trace newTrace {cobo, asad, aget, channel, pad};
            newTrace.AppendSample(tbid, sample);
            traces.emplace(hash,std::move(newTrace));
        }
        else {
            // The trace already exists. Append this sample to it.
            foundTrace->second.AppendSample(tbid, sample);
        }
    }
}

uint32_t Event::Size() const
{
    // Size depends on what is written to disk. This is defined by
    // the stream insertion operator.
    
    uint32_t size = sizeof("EVT") + sizeof(uint32_t) + sizeof(eventId) + sizeof(eventTime) + sizeof(uint16_t);
    for (auto item : traces) {
        size += item.second.size();
    }
    return size;
}

std::ostream& operator<<(std::ostream& stream, const Event& event)
{
    uint32_t sizeOfEvent = event.Size();
    
    stream.write(event.magic, sizeof(event.magic));
    
    stream.write((char*) &sizeOfEvent, sizeof(sizeOfEvent));
    stream.write((char*) &event.eventId, sizeof(event.eventId));
    stream.write((char*) &event.eventTime, sizeof(event.eventTime));
    
    uint16_t nTraces = (uint16_t) event.traces.size();
    stream.write((char*) &nTraces, sizeof(nTraces));
    
    for (auto item : event.traces)
    {
        stream << item.second;
    }

    return stream;
}

Trace& Event::GetTrace(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel)
{
    auto index = CalculateHash(cobo, asad, aget, channel);
    return traces.at(index);  // could throw out_of_range
}

uint32_t Event::GetEventId() const
{
    return eventId;
}

uint32_t Event::GetEventTime() const
{
    return eventTime;
}

void Event::SetEventId(uint32_t eventId_in)
{
    eventId = eventId_in;
}

void Event::SetEventTime(uint32_t eventTime_in)
{
    eventTime = eventTime_in;
}

void Event::SubtractFPN()
{
    std::vector<uint8_t> fpn_channels {11,22,45,56};  // from AGET Docs
    
    for (int cobo = 0; cobo < 10; cobo++) {
        for (int asad = 0; asad < 4; asad++) {
            for (int aget = 0; aget < 4; aget++) {
                
                // Get the FPN channels and find the mean
                Trace mean_fpn {};
                int num_fpns = 0;
                
                for (auto ch : fpn_channels) {
                    try {
                        mean_fpn += GetTrace(cobo, asad, aget, ch);
                        num_fpns++;
                    }
                    catch (std::out_of_range& e) {
                        continue;
                    }
                }
                
                // Check if there's any FPN data. If not, skip the next part.
                
                if (mean_fpn.GetNumberOfTimeBuckets() == 0) continue;
                
                mean_fpn /= num_fpns;
                
                // Renormalize mean FPN to zero

                mean_fpn.RenormalizeToZero();
                
                // Now subtract this mean from the other channels, binwise.
                // This iteration includes the FPN channels.
                
                auto begin = traces.lower_bound(CalculateHash(cobo, asad, aget, 0));
                auto end = traces.upper_bound(CalculateHash(cobo, asad, aget, 68));
                
                for (auto pos = begin; pos != end; pos++) {
                    pos->second -= mean_fpn;
                }
                
                // Finally, kill the traces that represent the FPN, since
                // we don't need them for anything else
                
                for (auto ch : fpn_channels) {
                    traces.erase(CalculateHash(cobo, asad, aget, ch));
                }
            }
        }
    }
}