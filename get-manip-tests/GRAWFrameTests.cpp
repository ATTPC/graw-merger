//
//  GRAWFrameTests.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/16/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "gtest/gtest.h"
#include "GRAWFrame.h"
#include "FakeRawFrame.h"
#include "MockGRAWFile.h"

#include <vector>
#include <iostream>

class GRAWFrameTestFixture : public testing::Test
{
protected:
    
    uint8_t GetMetaType(GRAWFrame& fr) {return fr.metaType;};
    uint32_t GRAWFrameSize(GRAWFrame& fr) {return fr.frameSize;};
    uint16_t GetHeaderSize(GRAWFrame& fr) {return fr.headerSize;};
    uint16_t GetItemSize(GRAWFrame& fr) {return fr.itemSize;};
    uint32_t GetNItems(GRAWFrame& fr) {return fr.nItems;};
    uint64_t GetEventTime(GRAWFrame& fr) {return fr.eventTime;};
    uint32_t GetEventId(GRAWFrame& fr) {return fr.eventId;};
    uint8_t  GetCoboId(GRAWFrame& fr) {return fr.coboId;};
    uint8_t  GetAsadId(GRAWFrame& fr) {return fr.asadId;};
    
    unsigned long GetSizeOfDataVec(GRAWFrame& fr) {return fr.data.size();};
    
public:
    
    MockGRAWFile mockdf;
    void TestConstructor(FakeRawFrame& fr, uint8_t cobo, uint8_t asad);
};

TEST(GRAWFrameTests,ExtractAgetId)
{
    uint32_t testData;
    for (int i = 0; i < 4; i++) {
        testData = i << 30;
        ASSERT_EQ(i,GRAWFrame::ExtractAgetId(testData));
    }
}

TEST(GRAWFrameTests,ExtractChannel)
{
    uint32_t testData;
    for (int i = 0; i < 68; i++) {
        testData = i << 23;
        ASSERT_EQ(i,GRAWFrame::ExtractChannel(testData));
    }
}

TEST(GRAWFrameTests,ExtractTBid)
{
    uint32_t testData;
    for (int i = 0; i < 512; i++) {
        testData = i << 14;
        ASSERT_EQ(i,GRAWFrame::ExtractTBid(testData));
    }
}

TEST(GRAWFrameTests,ExtractSample)
{
    uint32_t testData;
    for (int i = 0; i < 4095; i++) {
        testData = i;
        ASSERT_EQ(i,GRAWFrame::ExtractSample(testData));
    }
}

TEST(GRAWFrameTests,DISABLED_DataExtractionCombined)
{
    uint32_t testData;
    for (uint8_t aget = 0; aget < 4; aget++) {
        for (uint8_t ch = 0; ch < 68; ch++) {
            for (uint16_t tb = 0; tb < 512; tb++) {
                for (int16_t sample = 0; sample < 4095; sample++) {
                    testData = (aget << 30) | (ch << 23) | (tb << 14) | sample;
                    uint8_t aget_res = GRAWFrame::ExtractAgetId(testData);
                    uint8_t ch_res = GRAWFrame::ExtractChannel(testData);
                    uint16_t tb_res = GRAWFrame::ExtractTBid(testData);
                    int16_t sample_res = GRAWFrame::ExtractSample(testData);
                    ASSERT_EQ(aget, aget_res);
                    ASSERT_EQ(ch, ch_res);
                    ASSERT_EQ(tb, tb_res);
                    ASSERT_EQ(sample, sample_res);
                }
            }
        }
    }
}

void GRAWFrameTestFixture::TestConstructor(FakeRawFrame& fr, uint8_t cobo, uint8_t asad)
{
    auto fakeData = fr.GenerateRawFrameVector();
    
    GRAWFrame frame {fakeData, cobo, asad};
    EXPECT_EQ(fr.metatype,GetMetaType(frame));
    EXPECT_EQ(fr.frameSize,GRAWFrameSize(frame));
    EXPECT_EQ(fr.headerSize,GetHeaderSize(frame));
    EXPECT_EQ(fr.itemSize, GetItemSize(frame));
    EXPECT_EQ(fr.nItems, GetNItems(frame));
    EXPECT_EQ(fr.eventTime, GetEventTime(frame));
    EXPECT_EQ(fr.eventId, GetEventId(frame));
    EXPECT_EQ(fr.coboId, GetCoboId(frame));
    EXPECT_EQ(fr.asadId, GetAsadId(frame));
    EXPECT_EQ(fr.nItems, GetSizeOfDataVec(frame));
}

TEST_F(GRAWFrameTestFixture, Constructor)
{
    FakeRawFrame fr {1234567890, 12, 3, 2};
    TestConstructor(fr, 3, 2);
}

TEST_F(GRAWFrameTestFixture, Constructor_BadCobo)
{
    FakeRawFrame fakeData {1234567890, 12, 0, 2};
    
    GRAWFrame frame {fakeData.GenerateRawFrameVector(), 3, 2};
    ASSERT_EQ(GetCoboId(frame), 3);
}

TEST_F(GRAWFrameTestFixture, Constructor_BadAsad)
{
    FakeRawFrame fakeData {1234567890, 12, 3, 0};
    
    GRAWFrame frame {fakeData.GenerateRawFrameVector(), 3, 2};
    ASSERT_EQ(GetAsadId(frame), 2);
}

