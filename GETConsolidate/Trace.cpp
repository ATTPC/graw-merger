//
//  Trace.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "Trace.h"
#include <stdexcept>

Trace::Trace()
: coboId(0),asadId(0),agetId(0),channel(0),padId(0)
{}


Trace::Trace(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t ch, uint16_t pad)
: coboId(cobo), asadId(asad), agetId(aget), channel(ch), padId(pad)
{
}

void Trace::AppendSample(int tBucket, int sample)
{
    data.emplace(tBucket, sample);
}

int16_t Trace::GetSample(int tBucket) const
{
    return data.at(tBucket);
}

uint32_t Trace::Size()
{
    // Size depends on what is recorded. This is set in the
    // stream insertion operator for the trace.
    uint32_t size = sizeof(coboId) + sizeof(asadId) + sizeof(agetId) + sizeof(channel) + sizeof(padId) + sizeof(uint16_t) + uint32_t(data.size())*(sizeof(uint16_t)+sizeof(int16_t));
    return size;
}

unsigned long Trace::GetNumberOfTimeBuckets()
{
    return data.size();
}

Trace& Trace::operator+=(Trace& other)
{
    for (auto& other_item : other.data) {
        if (this->data.find(other_item.first) != this->data.end()) {
            this->data.at(other_item.first) += other_item.second;
        }
        else {
            this->data.insert(other_item);
        }
    }
    return *this;
}

Trace& Trace::operator-=(Trace& other)
{
    for (auto& other_item : other.data) {
        if (this->data.find(other_item.first) != this->data.end()) {
            this->data.at(other_item.first) -= other_item.second;
        }
        else {
            this->data.emplace(other_item.first,other_item.second * -1);
        }
    }
    return *this;
}

Trace& Trace::operator/=(Trace& other)
{
    for (auto& other_item : other.data) {
        if (this->data.find(other_item.first) != this->data.end()) {
            this->data.at(other_item.first) /= other_item.second;
        }
    }
    return *this;
}

Trace& Trace::operator/=(int i)
{
    for (auto& item : data) {
        item.second /= i;
    }
    return *this;
}

void Trace::RenormalizeToZero()
{
    int32_t total = std::accumulate(data.begin(),data.end(),0,
                                    [](int i, const std::pair<uint16_t,int16_t> p){return i + p.second;});
    total /= data.size();
    
    for (auto& el : data) {
        el.second -= total;
    }
}

uint32_t Trace::CompactSample(uint16_t tb, int16_t val)
{
    // val is 12-bits and tb is 9 bits. Fit this in 24 bits.
    uint32_t joined = (tb << 15) | val;
    return joined;
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
//        stream.write((char*) &(item.first), sizeof(item.first));
//        stream.write((char*) &(item.second), sizeof(item.second));
        auto compacted_data = Trace::CompactSample(item.first, item.second);
        stream.write((char*) &compacted_data ,3*sizeof(char));
    }

    return stream;
}
