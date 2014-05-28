//
//  GETFrameTests.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/16/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "gtest/gtest.h"
#include "GETFrame.h"
#include "FakeRawFrame.h"
#include "MockGRAWFile.h"

#include <vector>
#include <iostream>

class GETFrameTestFixture : public testing::Test
{
protected:
    
    uint8_t GetMetaType(GETFrame& fr) {return fr.metaType;};
    uint32_t GetFrameSize(GETFrame& fr) {return fr.frameSize;};
    uint16_t GetHeaderSize(GETFrame& fr) {return fr.headerSize;};
    uint16_t GetItemSize(GETFrame& fr) {return fr.itemSize;};
    uint32_t GetNItems(GETFrame& fr) {return fr.nItems;};
    uint64_t GetEventTime(GETFrame& fr) {return fr.eventTime;};
    uint32_t GetEventId(GETFrame& fr) {return fr.eventId;};
    uint8_t  GetCoboId(GETFrame& fr) {return fr.coboId;};
    uint8_t  GetAsadId(GETFrame& fr) {return fr.asadId;};
    
    unsigned long GetSizeOfDataVec(GETFrame& fr) {return fr.data.size();};
    
public:
    
    MockGRAWFile mockdf;
    void TestConstructor(FakeRawFrame& fr, uint8_t cobo, uint8_t asad);
};

TEST(GETFrameTests,ExtractAgetId)
{
    uint32_t testData;
    for (int i = 0; i < 4; i++) {
        testData = i << 30;
        ASSERT_EQ(i,GETFrame::ExtractAgetId(testData));
    }
}

TEST(GETFrameTests,ExtractChannel)
{
    uint32_t testData;
    for (int i = 0; i < 68; i++) {
        testData = i << 23;
        ASSERT_EQ(i,GETFrame::ExtractChannel(testData));
    }
}

TEST(GETFrameTests,ExtractTBid)
{
    uint32_t testData;
    for (int i = 0; i < 512; i++) {
        testData = i << 14;
        ASSERT_EQ(i,GETFrame::ExtractTBid(testData));
    }
}

TEST(GETFrameTests,ExtractSample)
{
    uint32_t testData;
    for (int i = 0; i < 4095; i++) {
        testData = i;
        ASSERT_EQ(i,GETFrame::ExtractSample(testData));
    }
}

TEST(GETFrameTests,DISABLED_DataExtractionCombined)
{
    uint32_t testData;
    for (uint8_t aget = 0; aget < 4; aget++) {
        for (uint8_t ch = 0; ch < 68; ch++) {
            for (uint16_t tb = 0; tb < 512; tb++) {
                for (int16_t sample = 0; sample < 4095; sample++) {
                    testData = (aget << 30) | (ch << 23) | (tb << 14) | sample;
                    uint8_t aget_res = GETFrame::ExtractAgetId(testData);
                    uint8_t ch_res = GETFrame::ExtractChannel(testData);
                    uint16_t tb_res = GETFrame::ExtractTBid(testData);
                    int16_t sample_res = GETFrame::ExtractSample(testData);
                    ASSERT_EQ(aget, aget_res);
                    ASSERT_EQ(ch, ch_res);
                    ASSERT_EQ(tb, tb_res);
                    ASSERT_EQ(sample, sample_res);
                }
            }
        }
    }
}

void GETFrameTestFixture::TestConstructor(FakeRawFrame& fr, uint8_t cobo, uint8_t asad)
{
    auto fakeData = fr.GenerateRawFrameVector();
    
    GETFrame frame {fakeData, cobo, asad};
    EXPECT_EQ(fr.metatype,GetMetaType(frame));
    EXPECT_EQ(fr.frameSize,GetFrameSize(frame));
    EXPECT_EQ(fr.headerSize,GetHeaderSize(frame));
    EXPECT_EQ(fr.itemSize, GetItemSize(frame));
    EXPECT_EQ(fr.nItems, GetNItems(frame));
    EXPECT_EQ(fr.eventTime, GetEventTime(frame));
    EXPECT_EQ(fr.eventId, GetEventId(frame));
    EXPECT_EQ(fr.coboId, GetCoboId(frame));
    EXPECT_EQ(fr.asadId, GetAsadId(frame));
    EXPECT_EQ(fr.nItems, GetSizeOfDataVec(frame));
}

TEST_F(GETFrameTestFixture, Constructor)
{
    FakeRawFrame fr {1234567890, 12, 3, 2};
    TestConstructor(fr, 3, 2);
}

TEST_F(GETFrameTestFixture, Constructor_BadCobo)
{
    FakeRawFrame fakeData {1234567890, 12, 0, 2};
    
    GETFrame frame {fakeData.GenerateRawFrameVector(), 3, 2};
    ASSERT_EQ(GetCoboId(frame), 3);
}

TEST_F(GETFrameTestFixture, Constructor_BadAsad)
{
    FakeRawFrame fakeData {1234567890, 12, 3, 0};
    
    GETFrame frame {fakeData.GenerateRawFrameVector(), 3, 2};
    ASSERT_EQ(GetAsadId(frame), 2);
}

