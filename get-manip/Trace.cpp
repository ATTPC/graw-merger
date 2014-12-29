//
//  Trace.cpp
//  get-manip
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "Trace.h"

// --------
// Static constants
// --------

const uint32_t Trace::sampleSize = 3;

// --------
// Constructors, Copy, and Move
// --------

Trace::Trace()
: coboId(0),asadId(0),agetId(0),channel(0),padId(0)
{
}


Trace::Trace(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t ch, uint16_t pad)
: coboId(cobo), asadId(asad), agetId(aget), channel(ch), padId(pad)
{
    assert(cobo < 10 && cobo >= 0);
    assert(asad < 4 && asad >= 0);
    assert(aget < 4 && aget >= 0);
    assert(ch < 68 && ch >= 0);
    assert((pad <= 10240 && pad >= 0) || pad == 20000);
}

Trace::Trace(const std::vector<uint8_t>& raw)
{
    auto pos = raw.begin();
    
    uint32_t traceSize = Utilities::ExtractInt<uint32_t>(pos, pos+sizeof(uint32_t));
    pos += sizeof(uint32_t);
    
    coboId = Utilities::ExtractInt<decltype(coboId)>(pos, pos+sizeof(coboId));
    if (coboId < 0 or coboId > 10) {
        throw Exceptions::Bad_Data();
    }
    pos += sizeof(coboId);
    
    asadId = Utilities::ExtractInt<decltype(asadId)>(pos, pos+sizeof(asadId));
    if (asadId < 0 or asadId > 3) {
        throw Exceptions::Bad_Data();
    }
    pos += sizeof(asadId);
    
    agetId = Utilities::ExtractInt<decltype(agetId)>(pos, pos+sizeof(agetId));
    if (agetId < 0 or agetId > 3) {
        throw Exceptions::Bad_Data();
    }
    pos += sizeof(agetId);
    
    channel = Utilities::ExtractInt<decltype(agetId)>(pos, pos+sizeof(channel));
    if (channel < 0 or channel > 68) {
        throw Exceptions::Bad_Data();
    }
    pos += sizeof(channel);
    
    padId = Utilities::ExtractInt<decltype(padId)>(pos, pos+sizeof(padId));
    pos += sizeof(padId);
    
    for (; pos != raw.end(); pos += Trace::sampleSize) {
        uint32_t item_packed = Utilities::ExtractInt<decltype(item_packed)>(pos, pos+Trace::sampleSize);
        auto item = Trace::UnpackSample(item_packed);
        data.at(item.first) = item.second;
    }
}

Trace::Trace(const Trace& orig)
: coboId(orig.coboId),asadId(orig.asadId),agetId(orig.agetId),channel(orig.channel),padId(orig.padId)
{
    this->data = orig.data;
}

Trace::Trace(Trace&& orig)
: coboId(orig.coboId),asadId(orig.asadId),agetId(orig.agetId),channel(orig.channel),padId(orig.padId)
{
    this->data = std::move(orig.data);
}

Trace& Trace::operator=(const Trace& orig)
{
    coboId = orig.coboId;
    asadId = orig.asadId;
    agetId = orig.agetId;
    channel = orig.channel;
    padId = orig.padId;
    
    data.clear();
    data = orig.data;
    return *this;
}

Trace& Trace::operator=(Trace&& orig)
{
    coboId = orig.coboId;
    asadId = orig.asadId;
    agetId = orig.agetId;
    channel = orig.channel;
    padId = orig.padId;
    
    data.clear();
    data = std::move(orig.data);
    return *this;
}

// --------
// Adding and Getting Data Items
// --------

void Trace::AppendSample(int tBucket, int sample)
{
    data.emplace(tBucket,sample);
}

int16_t Trace::GetSample(int tBucket) const
{
    return data.at(tBucket);
}

// --------
// Functions for Getting Size Information
// --------

uint32_t Trace::size() const
{
    // Size depends on what is recorded. This is set in the
    // stream insertion operator for the trace.
    
    // The size should not include the zeros that are not written to disk.
    
    uint32_t size = sizeof(uint32_t) + sizeof(coboId) + sizeof(asadId) + sizeof(agetId) + sizeof(channel) + sizeof(padId);
    
    size += Trace::sampleSize * data.size();
    
    return size;
}

unsigned long Trace::GetNumberOfTimeBuckets()
{
    return data.size();
}

// --------
// Operations on Contained Data
// --------

Trace& Trace::operator+=(Trace& other)
{
    for (auto& item : this->data) {
        try {
            item.second += other.data.at(item.first);
        }
        catch (std::out_of_range& e) {
            continue;
        }
    }
    return *this;
}

Trace& Trace::operator-=(Trace& other)
{
    for (auto& item : this->data) {
        try {
            item.second -= other.data.at(item.first);
        }
        catch (std::out_of_range& e) {
            continue;
        }
    }
    return *this;
}

Trace& Trace::operator-=(const sample_t i)
{
    for (auto& item : data) {
        item.second -= i;
    }
    return *this;
}

Trace& Trace::operator/=(Trace& other)
{
    for (auto& item : this->data) {
        try {
            auto div = other.data.at(item.first);
            if (div != 0) {
                item.second /= div;
            }
        }
        catch (std::out_of_range& e) {
            continue;
        }
    }
    return *this;
}

Trace& Trace::operator/=(int i)
{
    assert(i != 0);
    
    for (auto& item : data) {
        item.second /= i;
    }
    return *this;
}

void Trace::RenormalizeToZero()
{
    if (data.empty()) throw Exceptions::No_Data();
    
    int64_t total = 0;
    
    for (const auto& item : data) {
        total += item.second;
    }

    total /= data.size();
    
    for (auto& item : data) {
        item.second -= total;
    }
}

void Trace::ApplyThreshold(const sample_t threshold)
{
    for (auto& item : data) {
        if (item.second < threshold) {
            item.second = 0;
        }
    }
}

// --------
// I/O Operations
// --------

std::ostream& operator<<(std::ostream& stream, const Trace& trace)
{
    uint32_t size = trace.size();
    
    stream.write((char*) &size, sizeof(size));
    stream.write((char*) &(trace.coboId), sizeof(trace.coboId));
    stream.write((char*) &(trace.asadId), sizeof(trace.asadId));
    stream.write((char*) &(trace.agetId), sizeof(trace.agetId));
    stream.write((char*) &(trace.channel), sizeof(trace.channel));
    stream.write((char*) &(trace.padId), sizeof(trace.padId));
    
    for (const auto& item : trace.data)
    {
        auto compacted_data = Trace::CompactSample(item.first, item.second);
        stream.write((char*) &compacted_data ,Trace::sampleSize);
    }
    
    return stream;
}

// --------
// Private Data Compression Functions
// --------

uint32_t Trace::CompactSample(uint16_t tb, int16_t val)
{
    // val is 12-bits and tb is 9 bits. Fit this in 24 bits.
    // Use one bit for parity
    
    uint16_t narrowed = 0;
    uint16_t parity = 0;
    
    if (val < 0) {
        parity = (1 << 12);
        narrowed = -1*val;
    }
    else if (val >= 4095) {
        narrowed = 4095;
    }
    else {
        narrowed = val;
    }
    
    uint32_t joined = (tb << 15) | narrowed | parity;
    return joined;
}

std::pair<uint16_t,int16_t> Trace::UnpackSample(const uint32_t packed)
{
    uint16_t tb = (packed & 0xFF8000) >> 15;
    int16_t val = packed & 0xFFF;
    int16_t parity = (packed & 0x1000) >> 12;
    
    if (parity == 1) {
        val *= -1;
    }
    
    std::pair<uint16_t,int16_t> res {tb,val};
    return res;
}
