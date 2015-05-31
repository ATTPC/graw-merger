//
//  Event.cpp
//  get-manip
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "Event.h"

using namespace getevt;

// --------
// Static Variables
// --------

const uint8_t getevt::Event::magic {0xEE};

// --------
// Constructors, Move, and Copy
// --------

getevt::Event::Event()
: eventId(0),eventTime(0),lookupTable(nullptr),nFramesAppended(0)
{
}

getevt::Event::Event(const std::vector<uint8_t>& raw)
: eventId(0),eventTime(0),lookupTable(nullptr),nFramesAppended(0)
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
        try {
            Trace newTrace {rawTrace};
            auto newHash = CalculateHash(newTrace.coboId, newTrace.asadId, newTrace.agetId, newTrace.channel);
            traces.emplace(newHash,newTrace);
        }
        catch (std::exception& e) {
            std::cout << "Failed to read trace. Error:" << e.what() << std::endl;
        }
        
        // Increment the iterator
        rawIter += traceSize;
    }
}

getevt::Event::Event(const Event& orig)
: lookupTable(orig.lookupTable),eventId(orig.eventId),eventTime(orig.eventTime),nFramesAppended(orig.nFramesAppended)
{
    traces = orig.traces;
}

getevt::Event::Event(Event&& orig)
: lookupTable(orig.lookupTable),eventId(orig.eventId),eventTime(orig.eventTime),nFramesAppended(orig.nFramesAppended)
{
    traces = std::move(orig.traces);
}

Event& getevt::Event::operator=(const Event& orig)
{
    this->lookupTable = orig.lookupTable;  // could leak
    this->eventId = orig.eventId;
    this->eventTime = orig.eventTime;
    this->nFramesAppended = orig.nFramesAppended;
    
    this->traces.clear();
    this->traces = orig.traces;
    
    return *this;
}

Event& getevt::Event::operator=(Event&& orig)
{
    this->lookupTable = orig.lookupTable;  // could leak
    this->eventId = orig.eventId;
    this->eventTime = orig.eventTime;
    this->nFramesAppended = orig.nFramesAppended;
    
    this->traces.clear();
    this->traces = std::move(orig.traces);
    
    return *this;
}

// --------
// Setting Properties
// --------

void getevt::Event::SetLookupTable(PadLookupTable * table)
{
    lookupTable = table;
}

void getevt::Event::SetEventId(const evtid_t eventId_in)
{
    eventId = eventId_in;
}

void getevt::Event::SetEventTime(const ts_t eventTime_in)
{
    eventTime = eventTime_in;
}

void getevt::Event::AppendFrame(const GRAWFrame& frame)
{
    // Make sure pointers to required objects are valid

    if (lookupTable == NULL) {
        std::cerr << "No lookup table provided to Event." << std::endl;
        throw Exceptions::Not_Init();
    }
    
    // Get header information from frame
    
    addr_t cobo = frame.coboId;
    addr_t asad = frame.asadId;
    
    if (nFramesAppended == 0) {
        this->eventId = frame.eventId;
    }
    else if (this->eventId != frame.eventId) {
        std::cerr << "Event ID mismatch: CoBo " << (int) cobo << ", AsAd " << (int) asad << std::endl;
    }
    
//    long delta = static_cast<int64_t> (this->eventTime - frame.eventTime);
    
    if (nFramesAppended == 0) {
        this->eventTime = frame.eventTime;
    }
//    else if (labs(delta) > 10000) {
        // labs = long abs
        
//        LOG_WARNING << "Large event time mismatch. Event " << eventId << ", CoBo " << (int) cobo << ", AsAd, " << (int) asad << ". Delta " << long(this->eventTime) - long(frame.eventTime) <<  std::endl;
//    }
    
    nFramesAppended++;
    
    // Extract data items and create traces for them
    
    for (const auto& dataItem : frame.data) {
        // Extract information
        auto aget = dataItem.GetAgetId();
        auto channel = dataItem.GetChannel();
        auto tbid = dataItem.GetTimeBucketId();
        auto sample = dataItem.GetSample();
        
        auto hash = CalculateHash(cobo, asad, aget, channel);
        auto pad = lookupTable->Find(cobo, asad, aget, channel);
        
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

std::vector<GRAWFrame> getevt::Event::ExtractAllFrames()
{
    std::vector<GRAWFrame> frames;
    
    for (addr_t cobo = 0; cobo < Constants::num_cobos; cobo++) {
        for (addr_t asad = 0; asad < Constants::num_asads; asad++) {

            GRAWFrame fr {};
            fr.coboId = cobo;
            fr.asadId = asad;
            fr.eventTime = eventTime;
            fr.eventId = eventId;
            
            for (addr_t aget = 0; aget < Constants::num_agets; aget++) {
                for (addr_t ch = 0; ch < Constants::num_channels; ch++) {
                    auto tr = traces.find(CalculateHash(cobo, asad, aget, ch));
                    if (tr == traces.end()) continue;
                    
                    for (int i = 0; i < tr->second.data.size(); i++) {
                        // Gets all of the tbuckets for this channel
                        fr.data.push_back(GRAWDataItem(tr->second.agetId,
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

uint32_t getevt::Event::Size() const
{
    // Size depends on what is written to disk. This is defined by
    // the stream insertion operator.
    
    uint32_t size = sizeof(Event::magic) + sizeof(uint32_t) + sizeof(eventId) + sizeof(eventTime) + sizeof(uint16_t);
    for (const auto& item : traces) {
        size += item.second.size();
    }
    return size;
}

Trace& getevt::Event::GetTrace(addr_t cobo, addr_t asad, addr_t aget, addr_t channel)
{
    auto index = CalculateHash(cobo, asad, aget, channel);
    return traces.at(index);  // could throw out_of_range
}

evtid_t getevt::Event::GetEventId() const
{
    return eventId;
}

ts_t getevt::Event::GetEventTime() const
{
    return eventTime;
}

// --------
// Manipulation of Contained Data
// --------

void getevt::Event::SubtractFPN()
{
    std::vector<uint8_t> fpn_channels {11,22,45,56};  // from AGET Docs
    
    for (addr_t cobo = 0; cobo < Constants::num_cobos; cobo++) {
        for (addr_t asad = 0; asad < Constants::num_asads; asad++) {
            for (addr_t aget = 0; aget < Constants::num_agets; aget++) {
                
                // Get the FPN channels and find the mean.
                // Each FPN channel may be missing different time buckets, so
                // count the denom of the mean separately for each TB.
                
                Trace mean_fpn {};
                for (tb_t tb = 0; tb < Constants::num_tbs; tb++) {
                    mean_fpn.AppendSample(tb, 0);
                }
                
                std::vector<int> tb_multip (Constants::num_tbs,0);
                int num_fpns = 0;
                
                for (auto ch : fpn_channels) {
                    try {
                        auto& tr = GetTrace(cobo, asad, aget, ch);
                        mean_fpn += tr;
                        for (const auto& item : tr.data) {
                            tb_multip.at(item.first)++;
                        }
                        num_fpns++;
                    }
                    catch (std::out_of_range& e) {
                        continue;
                    }
                }
                
                // Check if there's any FPN data. If not, skip the next part.
                
                if (num_fpns == 0) continue;
                
                for (auto& item : mean_fpn.data) {
                    if (tb_multip.at(item.first) != 0) {
                        item.second /= tb_multip.at(item.first);
                    }
                }
                
                // Renormalize mean FPN to zero

                mean_fpn.RenormalizeToZero();
                
                // Now subtract this mean from the other channels, binwise.
                // This iteration includes the FPN channels.
                
//                auto begin = traces.lower_bound(CalculateHash(cobo, asad, aget, 0));
//                auto end = traces.upper_bound(CalculateHash(cobo, asad, aget, 68));
                
                for (addr_t ch = 0; ch < Constants::num_channels; ch++) {
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

void getevt::Event::SubtractPedestals(const LookupTable<sample_t>& pedsTable)
{
    for (auto& trace : traces) {
        auto cobo = trace.second.coboId;
        auto asad = trace.second.asadId;
        auto aget = trace.second.agetId;
        auto channel = trace.second.channel;
        
        auto pedValue = pedsTable.Find(cobo, asad, aget, channel);
        
        if (pedValue != 0) {
            trace.second -= pedValue;
        }
        else {
            continue;
        }
    }
}

void getevt::Event::ApplyThreshold(const sample_t threshold)
{
    for (auto& item : traces) {
        item.second.ApplyThreshold(threshold);
    }
}

void getevt::Event::DropZeros()
{
    for (auto trIter = traces.begin(); trIter != traces.end();) {
        trIter->second.DropZeros();
        if (trIter->second.Empty()) {
            trIter = traces.erase(trIter);
        }
        else {
            trIter++;
        }
    }
}

// --------
// I/O Functions
// --------

namespace getevt
{
    std::ostream& operator<<(std::ostream& stream, const getevt::Event& event)
    {
        uint32_t sizeOfEvent = event.Size();

        stream.write((char *) &(getevt::Event::magic), sizeof(getevt::Event::magic));

        stream.write((char *) &sizeOfEvent, sizeof(sizeOfEvent));
        stream.write((char *) &event.eventId, sizeof(event.eventId));
        stream.write((char *) &event.eventTime, sizeof(event.eventTime));

        uint16_t nTraces = (uint16_t) event.traces.size();
        stream.write((char *) &nTraces, sizeof(nTraces));

        for (const auto& item : event.traces) {
            stream << item.second;
        }

        return stream;
    }
}

// --------
// Private Functions
// --------

hash_t getevt::Event::CalculateHash(addr_t cobo, addr_t asad, addr_t aget, addr_t channel)
{
    // Widen the integers so they don't overflow on multiplication
    auto wcobo = hash_t(cobo);
    auto wasad = hash_t(asad);
    auto waget = hash_t(aget);
    auto wchannel = hash_t(channel);
    
    hash_t result = wchannel + waget*100 + wasad*10000 + wcobo*1000000;
    
    return result;
}