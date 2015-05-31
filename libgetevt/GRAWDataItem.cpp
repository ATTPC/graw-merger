//
//  GRAWDataItem.cpp
//  get-manip
//
//  Created by Joshua Bradt on 5/5/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "GRAWDataItem.h"

using namespace getevt;

// --------
// Constructor
// --------

GRAWDataItem::GRAWDataItem(addr_t agetId_in, addr_t channel_in,
                           tb_t timeBucketId_in, sample_t sample_in)
: agetId(agetId_in),channel(channel_in),timeBucketId(timeBucketId_in),
    sample(sample_in)
{
}

// --------
// Getters
// --------

addr_t GRAWDataItem::GetAgetId() const
{
    return this->agetId;
}

addr_t GRAWDataItem::GetChannel() const
{
    return this->channel;
}

tb_t GRAWDataItem::GetTimeBucketId() const
{
    return this->timeBucketId;
}

sample_t GRAWDataItem::GetSample() const
{
    return this->sample;
}