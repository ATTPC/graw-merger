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
#include "Constants.h"

class GRAWDataItem {
public:
    GRAWDataItem(addr_t agetId_in, addr_t channel_in,
                 tb_t timeBucketId_in, sample_t sample_in);
    
    addr_t GetAgetId() const;
    addr_t GetChannel() const;
    tb_t GetTimeBucketId() const;
    sample_t GetSample() const;
    
private:
    addr_t agetId;
    addr_t channel;
    tb_t timeBucketId;
    sample_t sample;
};

#endif /* defined(__get_manip__GRAWDataItem__) */
