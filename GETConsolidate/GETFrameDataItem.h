//
//  GETFrameDataItem.h
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/5/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __GETConsolidate__GETFrameDataItem__
#define __GETConsolidate__GETFrameDataItem__

#include <iostream>

class GETFrameDataItem {
public:
    GETFrameDataItem(uint8_t agetId_in, uint8_t channel_in,
                     uint16_t timeBucketId_in, float sample_in);
    
    uint8_t GetAgetId();
    uint8_t GetChannel();
    uint16_t GetTimeBucketId();
    float GetSample();
    
private:
    uint8_t agetId;
    uint8_t channel;
    uint16_t timeBucketId;
    float sample;
};

#endif /* defined(__GETConsolidate__GETFrameDataItem__) */
