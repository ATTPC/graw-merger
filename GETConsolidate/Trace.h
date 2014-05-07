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
#include <array>

class Trace
{
private:
    static const int nTimeBuckets = 512;
    
    uint8_t coboId;
    uint8_t asadId;
    uint8_t agetId;
    uint8_t channel;
    uint16_t padId;
    std::array<float,nTimeBuckets> *data;
    
public:
    Trace(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t ch, uint16_t pad);
    ~Trace();
    
    void AppendSample(int tBucket, float sample);
};

#endif /* defined(__GETConsolidate__Trace__) */
