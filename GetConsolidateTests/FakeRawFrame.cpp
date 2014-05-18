//
//  FakeRawFrame.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/17/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include <cmath>

#include "FakeRawFrame.h"

FakeRawFrame::FakeRawFrame(uint64_t eventTime_in, uint32_t eventId_in,
                           uint8_t cobo, uint8_t asad)
: eventTime(eventTime_in),eventId(eventId_in),coboId(cobo),asadId(asad)
{
    AppendFPN();
    UpdateSizes();
}

FakeRawFrame::FakeRawFrame()
: eventTime(0),eventId(0),coboId(0),asadId(0)
{
    AppendFPN();
    UpdateSizes();
}

void FakeRawFrame::AppendDataItem(uint32_t aget, uint32_t ch,
                                  uint32_t tb, uint32_t sample)
{
    // This will implicitly narrow some of these fields.
    // Also, they may overflow off the left if done wrong.
    
    uint32_t item {0};
    item |= sample;
    item |= (tb << 14);
    item |= (ch << 23);
    item |= (aget << 30);
    
    dataItems.push_back(item);
    UpdateSizes();
}

void FakeRawFrame::AppendFPN()
{
    std::vector<uint8_t> fpn_chans {12,23,46,57};
    
    for (uint8_t aget = 0; aget < 4; aget++) {
        for (auto ch : fpn_chans) {
            for (uint16_t tb = 0; tb < 512; tb++) {
                AppendDataItem(aget, ch, tb, ch*10);
            }
        }
    }
}

template<typename T>
void FakeRawFrame::AppendBytes(std::vector<uint8_t>& vec, T val, int nBytes)
{
    for (int i = nBytes-1; i >= 0; i--) {
        vec.push_back((val & (0xFF << i*8)) >> i*8);
    }
}

std::vector<uint8_t> FakeRawFrame::GenerateRawFrameVector()
{
    std::vector<uint8_t> res;
    res.push_back(metatype);
    AppendBytes(res, frameSize, 3);
    res.push_back(dataSource);
    AppendBytes(res, frameType, 2);
    res.push_back(revision);
    AppendBytes(res, headerSize, 2);
    AppendBytes(res, itemSize, 2);
    AppendBytes(res, nItems, 4);
    AppendBytes(res, eventTime, 6);
    AppendBytes(res, eventId, 4);
    res.push_back(coboId);
    res.push_back(asadId);
    AppendBytes(res, readOffset, 2);
    res.push_back(status);
    
    for (auto item : hitPatterns) {
        AppendBytes(res, item.to_ullong(), 9);
    }
    
    for (auto item : multiplicities) {
        AppendBytes(res, item, 2);
    }
    
    // Pad it out
    
    while (res.size() < headerSize * 64) {
        res.push_back(0x00);
    }
    
    for (auto item : dataItems) {
        AppendBytes(res, item, 4);
    }
    
    return res;
}

void FakeRawFrame::UpdateSizes()
{
    nItems = static_cast<uint32_t> (dataItems.size());
    
    // frame size is in units of 64 Bytes
    uint32_t rawSize = (headerSize * 64) + itemSize * nItems;
    frameSize = ceil(rawSize / 64);
}