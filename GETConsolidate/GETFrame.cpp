//
//  GETFrame.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/5/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "GETFrame.h"

uint16_t GETFrame::ExtractByteSwapInt16(std::vector<uint8_t>::iterator begin,
                              std::vector<uint8_t>::iterator end)
{
    uint16_t result = 0;
    int n = 0;
    for (auto iter = end-1; iter >= begin; iter--) {
        result |= (*iter)<<(8*n);
        n++;
    }
    return result;
}

GETFrame::GETFrame(std::vector<uint8_t> *rawFrame)
{
    auto rawFrameIter = rawFrame->begin();
    metaType = *rawFrameIter;
    rawFrameIter++;
    frameSize = ExtractByteSwapInt16(rawFrameIter, rawFrameIter + 3);
    rawFrameIter += 3;
    dataSource = *rawFrameIter;
    rawFrameIter++;
    
}