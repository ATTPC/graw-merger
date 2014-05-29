//
//  GRAWDataItem.h
//  get-manip
//
//  Created by Joshua Bradt on 5/5/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __get_manip__GRAWDataItem__
#define __get_manip__GRAWDataItem__

#include <iostream>

class GRAWDataItem {
public:
    GRAWDataItem(uint8_t agetId_in, uint8_t channel_in,
                     uint16_t timeBucketId_in, uint16_t sample_in);
    
    uint8_t GetAgetId() const;
    uint8_t GetChannel() const;
    uint16_t GetTimeBucketId() const;
    uint16_t GetSample() const;
    
private:
    uint8_t agetId;
    uint8_t channel;
    uint16_t timeBucketId;
    uint16_t sample;
};

#endif /* defined(__get_manip__GRAWDataItem__) */
