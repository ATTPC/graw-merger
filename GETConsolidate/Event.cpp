//
//  Event.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "Event.h"

// --------
// Static Variables
// --------

const uint8_t Event::magic {0xEE};

// --------
// Constructors, Move, and Copy
// --------

Event::Event()
{
}

Event::Event(const std::vector<uint8_t>& raw)
{
    auto rawIter = raw.begin();
    
    uint8_t magic_in = Utilities::ExtractInt<decltype(magic_in)>(rawIter, rawIter+sizeof(magic_in));
    rawIter += sizeof(magic_in);
    
    if (magic_in != Event::magic) throw Exceptions::Wrong_File_Position();
    
//    uint32_t sizeOfEvent = Utilities::ExtractInt<uint32_t>(rawIter, rawIter+4);
    rawIter += 4;
    
    eventId = Utilities::ExtractInt<decltype(eventId)>(rawIter, rawIter+sizeof(eventId));
    rawIter += sizeof(eventId);
    
    eventTime = Utilities::ExtractInt<decltype(eventTime)>(rawIter, rawIter+sizeof(eventTime));
    rawIter += sizeof(eventTime);
    
    uint16_t nTraces = Utilities::ExtractInt<uint16_t>(rawIter, rawIter+sizeof(uint16_t));
    rawIter += sizeof(uint16_t);
    
    for (decltype(nTraces) n = 0; n < nTraces; n++) {
        // Find trace size
        uint32_t traceSize = Utilities::ExtractInt<decltype(traceSize)>(rawIter, rawIter+sizeof(traceSize));
        std::vector<uint8_t> rawTrace {rawIter,rawIter+traceSize};
        
        // Create and emplace the new trace
        Trace newTrace {rawTrace};
        auto newHash = CalculateHash(newTrace.coboId, newTrace.asadId, newTrace.agetId, newTrace.channel);
        traces.emplace(newHash,newTrace);
        
        // Increment the iterator
        rawIter += traceSize;
    }
}

Event::Event(const Event& orig)
: lookupTable(orig.lookupTable),eventId(orig.eventId),eventTime(orig.eventTime)
{
    traces = orig.traces;
}

Event::Event(Event&& orig)
: lookupTable(orig.lookupTable),eventId(orig.eventId),eventTime(orig.eventTime)
{
    traces = std::move(orig.traces);
}

Event& Event::operator=(const Event& orig)
{
    this->lookupTable = orig.lookupTable;  // could leak
    this->eventId = orig.eventId;
    this->eventTime = orig.eventTime;
    
    this->traces.clear();
    this->traces = orig.traces;
    
    return *this;
}

Event& Event::operator=(Event&& orig)
{
    this->lookupTable = orig.lookupTable;  // could leak
    this->eventId = orig.eventId;
    this->eventTime = orig.eventTime;
    
    this->traces.clear();
    this->traces = std::move(orig.traces);
    
    return *this;
}

// --------
// Setting Properties
// --------

void Event::SetLookupTable(PadLookupTable * table)
{
    lookupTable = table;
}

void Event::SetEventId(const uint32_t eventId_in)
{
    eventId = eventId_in;
}

void Event::SetEventTime(const uint64_t eventTime_in)
{
    eventTime = eventTime_in;
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
    
    for (const auto& dataItem : frame.data) {
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

std::vector<GETFrame> Event::ExtractAllFrames()
{
    std::vector<GETFrame> frames;
    
    for (uint8_t cobo = 0; cobo < 10; cobo++) {
        for (uint8_t asad = 0; asad < 4; asad++) {

            GETFrame fr {};
            fr.coboId = cobo;
            fr.asadId = asad;
            fr.eventTime = eventTime;
            fr.eventId = eventId;
            
            for (uint8_t aget = 0; aget < 4; aget++) {
                for (uint8_t ch = 0; ch < 68; ch++) {
                    auto tr = traces.find(CalculateHash(cobo, asad, aget, ch));
                    if (tr == traces.end()) continue;
                    
                    for (int i = 0; i < tr->second.data.size(); i++) {
                        // Gets all of the tbuckets for this channel
                        fr.data.push_back(GETFrameDataItem(tr->second.agetId,
                                                           tr->second.channel,
                                                           i,
                                                           tr->second.data.at(i)));
                    }
                    fr.hitPatterns.at(aget).set(ch,1);
                }
                
            }

            fr.nItems = static_cast<uint32_t>(fr.data.size());
            fr.frameSize = fr.headerSize + ceil((fr.nItems*fr.itemSize)/64);
            
            if (fr.nItems != 0) {
                frames.push_back(fr);
            }
        }
    }
    
    return frames;
}

// --------
// Getting Properties and Members
// --------

uint32_t Event::Size() const
{
    // Size depends on what is written to disk. This is defined by
    // the stream insertion operator.
    
    uint32_t size = sizeof(Event::magic) + sizeof(uint32_t) + sizeof(eventId) + sizeof(eventTime) + sizeof(uint16_t);
    for (const auto& item : traces) {
        size += item.second.size();
    }
    return size;
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

uint64_t Event::GetEventTime() const
{
    return eventTime;
}

// --------
// Manipulation of Contained Data
// --------

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
                
                if (num_fpns == 0) continue;
                
                mean_fpn /= num_fpns;
                
                // Renormalize mean FPN to zero

                mean_fpn.RenormalizeToZero();
                
                // Now subtract this mean from the other channels, binwise.
                // This iteration includes the FPN channels.
                
//                auto begin = traces.lower_bound(CalculateHash(cobo, asad, aget, 0));
//                auto end = traces.upper_bound(CalculateHash(cobo, asad, aget, 68));
                
                for (auto ch = 0; ch < 68; ch++) {
                    auto tr = traces.find(CalculateHash(cobo, asad, aget, ch));
                    if (tr != traces.end()) {
                        tr->second -= mean_fpn;
                    }
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

// --------
// I/O Functions
// --------

std::ostream& operator<<(std::ostream& stream, const Event& event)
{
    uint32_t sizeOfEvent = event.Size();
    
    stream.write((char*) &(Event::magic), sizeof(Event::magic));
    
    stream.write((char*) &sizeOfEvent, sizeof(sizeOfEvent));
    stream.write((char*) &event.eventId, sizeof(event.eventId));
    stream.write((char*) &event.eventTime, sizeof(event.eventTime));
    
    uint16_t nTraces = (uint16_t) event.traces.size();
    stream.write((char*) &nTraces, sizeof(nTraces));
    
    for (const auto& item : event.traces)
    {
        stream << item.second;
    }
    
    return stream;
}

// --------
// Private Functions
// --------

int Event::CalculateHash(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel)
{
    return channel + aget*100 + asad*10000 + cobo*1000000;
}