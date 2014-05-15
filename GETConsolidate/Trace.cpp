//
//  Trace.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "Trace.h"
#include <stdexcept>

const uint32_t Trace::sampleSize = 3;

template<typename outType>
outType Trace::ExtractInt(std::vector<char>::const_iterator begin,
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

Trace::Trace()
: coboId(0),asadId(0),agetId(0),channel(0),padId(0)
{}


Trace::Trace(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t ch, uint16_t pad)
: coboId(cobo), asadId(asad), agetId(aget), channel(ch), padId(pad)
{
}

Trace::Trace(std::vector<char> raw)
{
    auto pos = raw.begin();
    
//    uint32_t traceSize = ExtractInt<uint32_t>(pos, pos+sizeof(uint32_t));
    pos += sizeof(uint32_t);
    
    coboId = ExtractInt<decltype(coboId)>(pos, pos+sizeof(coboId));
    if (coboId < 0 or coboId > 10) throw Exceptions::Bad_Data();
    pos += sizeof(coboId);
    
    asadId = ExtractInt<decltype(asadId)>(pos, pos+sizeof(asadId));
    if (asadId < 0 or asadId > 3) throw Exceptions::Bad_Data();
    pos += sizeof(asadId);
    
    agetId = ExtractInt<decltype(agetId)>(pos, pos+sizeof(agetId));
    if (agetId < 0 or agetId > 3) throw Exceptions::Bad_Data();
    pos += sizeof(agetId);
    
    channel = ExtractInt<decltype(agetId)>(pos, pos+sizeof(channel));
    if (channel < 0 or channel > 68) throw Exceptions::Bad_Data();
    pos += sizeof(channel);
    
    padId = ExtractInt<decltype(padId)>(pos, pos+sizeof(padId));
    pos += sizeof(padId);
    
    while (pos != raw.end()) {
        uint32_t item = ExtractInt<decltype(item)>(pos, pos+Trace::sampleSize);
        data.insert(Trace::UnpackSample(item));
        pos += Trace::sampleSize;
    }
}

void Trace::AppendSample(int tBucket, int sample)
{
    data.emplace(tBucket, sample);
}

int16_t Trace::GetSample(int tBucket) const
{
    return data.at(tBucket);
}

uint32_t Trace::size() const
{
    // Size depends on what is recorded. This is set in the
    // stream insertion operator for the trace.
    uint32_t size = sizeof(Trace::sampleSize) + sizeof(coboId) + sizeof(asadId) + sizeof(agetId) + sizeof(channel) + sizeof(padId) + uint32_t(data.size())*(Trace::sampleSize);
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

std::pair<uint16_t,int16_t> Trace::UnpackSample(const uint32_t packed)
{
    uint16_t tb = (packed & 0xFF8000) >> 15;
    int16_t val = packed & 0xFFF;
    std::pair<uint16_t,int16_t> res {tb,val};
    return res;
}

std::ostream& operator<<(std::ostream& stream, const Trace& trace)
{
    uint32_t size = trace.size();
    
    stream.write((char*) &size, sizeof(size));
    stream.write((char*) &(trace.coboId), sizeof(trace.coboId));
    stream.write((char*) &(trace.asadId), sizeof(trace.asadId));
    stream.write((char*) &(trace.agetId), sizeof(trace.agetId));
    stream.write((char*) &(trace.channel), sizeof(trace.channel));
    stream.write((char*) &(trace.padId), sizeof(trace.padId));
    
    for (auto item : trace.data)
    {
//        stream.write((char*) &(item.first), sizeof(item.first));
//        stream.write((char*) &(item.second), sizeof(item.second));
        auto compacted_data = Trace::CompactSample(item.first, item.second);
        stream.write((char*) &compacted_data ,3*sizeof(char));
    }

    return stream;
}
