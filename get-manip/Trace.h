//
//  Trace.h
//  get-manip
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __get_manip__Trace__
#define __get_manip__Trace__

#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <numeric>
#include <assert.h>
#include "GMExceptions.h"
#include "Utilities.h"
#include "Constants.h"

class Trace
{
public:
    
    // Constructors, copy, and move
    
    Trace();
    Trace(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t ch, uint16_t pad);
    Trace(const std::vector<uint8_t>& raw);
    Trace(const Trace& orig);
    Trace(Trace&& orig);
    
    Trace& operator=(const Trace& orig);
    Trace& operator=(Trace&& orig);
    
    // Adding and getting data items
    
    void AppendSample(int tBucket, int sample);
    int16_t GetSample(int tBucket) const;
    
    // Functions to get info about the size of the trace
    
    uint32_t size() const;
    unsigned long GetNumberOfTimeBuckets();
    bool Empty() const;
    
    // Operations on contained data
    
    Trace& operator+=(Trace& other);
    Trace& operator-=(Trace& other);
    Trace& operator-=(const sample_t i);
    Trace& operator/=(Trace& other);    // division by another trace
    Trace& operator/=(int i);           // division by an integer
    
    void RenormalizeToZero();
    void ApplyThreshold(const sample_t threshold);
    void DropZeros();
    
    // Constants
    
    static const uint32_t sampleSize;
    
    // I/O Operations
    
    friend std::ostream& operator<<(std::ostream& stream, const Trace& trace);
    
private:
    
    // Contained data
    
    uint8_t coboId;
    uint8_t asadId;
    uint8_t agetId; 
    uint8_t channel;
    uint16_t padId;
    std::unordered_map<uint16_t,int16_t> data;   // maps timebucket:sample
    
    // Private functions to compress data on file write
    
    static uint32_t CompactSample(uint16_t tb, int16_t val);
    static std::pair<uint16_t,int16_t> UnpackSample(const uint32_t packed);
    
    // Friends
    
    friend class Event;
    friend class TraceTestFixture;
    friend class EventTestFixture;
};

#endif /* defined(__get_manip__Trace__) */
