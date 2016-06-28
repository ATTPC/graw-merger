//
//  Event.cpp
//  get-manip
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "Event.h"

// --------
// Constructors, Move, and Copy
// --------

Event::Event()
: eventId(0),eventTime(0),lookupTable(nullptr),nFramesAppended(0)
{
}

Event::Event(const Event& orig)
: eventId(orig.eventId),eventTime(orig.eventTime),lookupTable(orig.lookupTable),nFramesAppended(orig.nFramesAppended)
{
    data = orig.data;
}

Event::Event(Event&& orig)
: eventId(orig.eventId),eventTime(orig.eventTime),lookupTable(orig.lookupTable),nFramesAppended(orig.nFramesAppended)
{
    data = std::move(orig.data);
}

Event& Event::operator=(const Event& orig)
{
    this->lookupTable = orig.lookupTable;  // could leak
    this->eventId = orig.eventId;
    this->eventTime = orig.eventTime;
    this->nFramesAppended = orig.nFramesAppended;

    this->data.clear();
    this->data = orig.data;

    return *this;
}

Event& Event::operator=(Event&& orig)
{
    this->lookupTable = orig.lookupTable;  // could leak
    this->eventId = orig.eventId;
    this->eventTime = orig.eventTime;
    this->nFramesAppended = orig.nFramesAppended;

    this->data.clear();
    this->data = std::move(orig.data);

    return *this;
}

Event::mapType::iterator Event::begin()
{
    return data.begin();
}

Event::mapType::iterator Event::end()
{
    return data.end();
}

Event::mapType::const_iterator Event::cbegin() const
{
    return data.cbegin();
}

Event::mapType::const_iterator Event::cend() const
{
    return data.cend();
}

// --------
// Setting Properties
// --------

void Event::SetLookupTable(PadLookupTable * table)
{
    lookupTable = table;
}

void Event::AppendFrame(const GRAWFrame& frame)
{
    // Make sure pointers to required objects are valid

    if (lookupTable == NULL) {
        LOG_ERROR << "No lookup table provided to Event." << std::endl;
        throw Exceptions::Not_Init();
    }

    // Get header information from frame

    addr_t cobo = frame.coboId;
    addr_t asad = frame.asadId;

    if (nFramesAppended == 0) {
        this->eventId = frame.eventId;
    }
    else if (this->eventId != frame.eventId) {
        LOG_WARNING << "Event ID mismatch: CoBo " << (int) cobo << ", AsAd " << (int) asad << std::endl;
    }

    if (nFramesAppended == 0) {
        this->eventTime = frame.eventTime;
    }

    nFramesAppended++;

    // Extract data items and create traces for them

    for (auto frameIter = frame.cbegin(); frameIter < frame.cend(); frameIter++) {
        const GRAWDataItem& dataItem = *frameIter;
        // Extract information
        auto aget = dataItem.agetId;
        auto channel = dataItem.channel;
        auto tbid = dataItem.timeBucketId;
        auto sample = dataItem.sample;

        auto pad = lookupTable->Find(cobo, asad, aget, channel);

        HardwareAddress hwaddr = {cobo, asad, aget, channel, pad};

        // Find trace in hash table, if it exists
        arma::Col<sample_t>& tr = data[hwaddr];

        if (tr.is_empty()) {
            // The trace was just default-constructed, so it's new
            tr.set_size(Constants::num_tbs);
        }

        tr(tbid) = sample;
    }
}

// --------
// Getting Properties and Members
// --------

arma::Col<sample_t>& Event::GetTrace(addr_t cobo, addr_t asad, addr_t aget, addr_t channel)
{
    HardwareAddress hwaddr {cobo, asad, aget, channel};
    return data.at(hwaddr);  // could throw out_of_range
}

size_t Event::numTraces() const
{
    return data.size();
}

// --------
// Manipulation of Contained Data
// --------

// void Event::SubtractFPN()
// {
//     std::vector<uint8_t> fpn_channels {11,22,45,56};  // from AGET Docs
//
//     for (addr_t cobo = 0; cobo < Constants::num_cobos; cobo++) {
//         for (addr_t asad = 0; asad < Constants::num_asads; asad++) {
//             for (addr_t aget = 0; aget < Constants::num_agets; aget++) {
//
//                 // Get the FPN channels and find the mean.
//                 // Each FPN channel may be missing different time buckets, so
//                 // count the denom of the mean separately for each TB.
//
//                 Trace mean_fpn {};
//                 for (tb_t tb = 0; tb < Constants::num_tbs; tb++) {
//                     mean_fpn.AppendSample(tb, 0);
//                 }
//
//                 std::vector<int> tb_multip (Constants::num_tbs,0);
//                 int num_fpns = 0;
//
//                 for (auto ch : fpn_channels) {
//                     try {
//                         auto& tr = GetTrace(cobo, asad, aget, ch);
//                         mean_fpn += tr;
//                         for (const auto& item : tr.data) {
//                             tb_multip.at(item.first)++;
//                         }
//                         num_fpns++;
//                     }
//                     catch (std::out_of_range& e) {
//                         continue;
//                     }
//                 }
//
//                 // Check if there's any FPN data. If not, skip the next part.
//
//                 if (num_fpns == 0) continue;
//
//                 for (auto& item : mean_fpn.data) {
//                     if (tb_multip.at(item.first) != 0) {
//                         item.second /= tb_multip.at(item.first);
//                     }
//                 }
//
//                 // Renormalize mean FPN to zero
//
//                 mean_fpn.RenormalizeToZero();
//
//                 // Now subtract this mean from the other channels, binwise.
//                 // This iteration includes the FPN channels.
//
// //                auto begin = traces.lower_bound(CalculateHash(cobo, asad, aget, 0));
// //                auto end = traces.upper_bound(CalculateHash(cobo, asad, aget, 68));
//
//                 for (addr_t ch = 0; ch < Constants::num_channels; ch++) {
//                     auto tr = traces.find(CalculateHash(cobo, asad, aget, ch));
//                     if (tr != traces.end()) {
//                         tr->second -= mean_fpn;
//                     }
//                 }
//
//                 // Finally, kill the traces that represent the FPN, since
//                 // we don't need them for anything else
//
//                 for (auto ch : fpn_channels) {
//                     traces.erase(CalculateHash(cobo, asad, aget, ch));
//                 }
//             }
//         }
//     }
// }

// void Event::SubtractPedestals(const LookupTable<sample_t>& pedsTable)
// {
//     for (auto& trace : traces) {
//         auto cobo = trace.second.coboId;
//         auto asad = trace.second.asadId;
//         auto aget = trace.second.agetId;
//         auto channel = trace.second.channel;
//
//         auto pedValue = pedsTable.Find(cobo, asad, aget, channel);
//
//         if (pedValue != 0) {
//             trace.second -= pedValue;
//         }
//         else {
//             continue;
//         }
//     }
// }
//
// void Event::ApplyThreshold(const sample_t threshold)
// {
//     for (auto& item : traces) {
//         item.second.ApplyThreshold(threshold);
//     }
// }
