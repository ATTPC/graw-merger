//
//  Trace.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "Trace.h"
#include <stdexcept>

Trace::Trace(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t ch, uint16_t pad)
: coboId(cobo), asadId(asad), agetId(aget), channel(ch), padId(pad)
{
}

void Trace::AppendSample(int tBucket, int sample)
{
    data.emplace(tBucket, sample);
}

uint32_t Trace::Size()
{
    // Size depends on what is recorded. This is set in the
    // stream insertion operator for the trace.
    uint32_t size = sizeof(coboId) + sizeof(asadId) + sizeof(agetId) + sizeof(channel) + sizeof(padId) + sizeof(uint16_t) + uint32_t(data.size())*(sizeof(uint16_t)+sizeof(int16_t));
    return size;
}

std::ostream& operator<<(std::ostream& stream, const Trace& trace)
{
    stream.write((char*) &(trace.coboId), sizeof(trace.coboId));
    stream.write((char*) &(trace.asadId), sizeof(trace.asadId));
    stream.write((char*) &(trace.agetId), sizeof(trace.agetId));
    stream.write((char*) &(trace.channel), sizeof(trace.channel));
    stream.write((char*) &(trace.padId), sizeof(trace.padId));
    
    // Number of time buckets
    
    uint16_t nTimeBuckets = trace.data.size();
    stream.write((char*) &nTimeBuckets, sizeof(nTimeBuckets));
    
    for (auto item : trace.data)
    {
        stream.write((char*) &(item.first), sizeof(item.first));
        stream.write((char*) &(item.second), sizeof(item.second));
    }

    return stream;
}