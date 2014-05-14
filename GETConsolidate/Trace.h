//
//  Trace.h
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __GETConsolidate__Trace__
#define __GETConsolidate__Trace__

#include <iostream>
#include <map>
#include <numeric>

class Trace
{
private:
    
    uint8_t coboId;
    uint8_t asadId;
    uint8_t agetId;
    uint8_t channel;
    uint16_t padId;
    std::map<uint16_t,int16_t> data;   // maps timebucket:sample
    
    friend class Event;
    
public:
    Trace();
    Trace(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t ch, uint16_t pad);
    
    Trace& operator/=(Trace& other);    // division by another trace
    Trace& operator/=(int i);           // division by an integer
    Trace& operator+=(Trace& other);
    Trace& operator-=(Trace& other);
    
    void RenormalizeToZero();
    
    void AppendSample(int tBucket, int sample);

    int16_t GetSample(int tBucket) const;
    
    uint32_t Size();
    
    unsigned long GetNumberOfTimeBuckets();
    
    static uint32_t CompactSample(uint16_t tb, int16_t val);
    
    friend std::ostream& operator<<(std::ostream& stream, const Trace& trace);
};

#endif /* defined(__GETConsolidate__Trace__) */
