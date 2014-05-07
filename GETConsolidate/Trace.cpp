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
    data = new std::array<float,nTimeBuckets>;
    data->fill(0);
}

Trace::~Trace()
{
    delete data;
}

void Trace::AppendSample(int tBucket, float sample)
{
    try {
        data->at(tBucket) = sample;
    }
    catch (const std::out_of_range& range_err) {
        std::cout << "Out of Range Error in Trace::AppendSample: " << range_err.what() << std::endl;
    }
}