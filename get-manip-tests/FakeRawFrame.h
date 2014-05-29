//
//  FakeRawFrame.h
//  get-manip
//
//  Created by Joshua Bradt on 5/17/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __get_manip__FakeRawFrame__
#define __get_manip__FakeRawFrame__

#include <vector>
#include <bitset>

class FakeRawFrame
{
public:
    uint8_t metatype {0x06};
    uint32_t frameSize {}; // actually 3 bytes
    uint8_t dataSource {0x0};
    uint16_t frameType {0x1};
    uint8_t revision {0x4};
    uint16_t headerSize {0x4};
    uint16_t itemSize {0x4};
    uint32_t nItems {};
    uint64_t eventTime {}; // actually 6 bytes
    uint32_t eventId {};
    uint8_t coboId {};
    uint8_t asadId {};
    uint16_t readOffset {0x0};
    uint8_t status {0x0};
    std::vector<std::bitset<9*8>> hitPatterns;
    std::vector<uint16_t> multiplicities;
    
    std::vector<uint32_t> dataItems;
    
    FakeRawFrame(uint64_t eventTime_in, uint32_t eventId_in,
                 uint8_t cobo, uint8_t asad);
    FakeRawFrame();
    
    void AppendDataItem(uint32_t aget, uint32_t ch, uint32_t tb, uint32_t sample);
    void AppendFPN();
    
    std::vector<uint8_t> GenerateRawFrameVector();
    
    template<typename T>
    void AppendBytes(std::vector<uint8_t>& vec, T val, int nBytes);
    
    void UpdateSizes();
};

#endif /* defined(__getmanip__FakeRawFrame__) */
