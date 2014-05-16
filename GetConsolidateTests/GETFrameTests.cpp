//
//  GETFrameTests.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/16/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "gtest/gtest.h"
#include "GETFrame.h"
#include "MockGETDataFile.h"

#include <vector>
#include <iostream>

class GETFrameTestFixture : public testing::Test
{
protected:
    GETFrameTestFixture()
    {
        std::ifstream file ("./MockData/GoodFrame.graw", std::ios::binary);
        while (file.good()) {
            char tmp;
            file.get(tmp);
            fakeData.push_back(tmp);
        }
        fakeData.pop_back();  // kludge to take care of duplication of last byte
        file.close();
    }
    
    uint8_t GetMetaType(GETFrame& fr) {return fr.metaType;};
    uint32_t GetFrameSize(GETFrame& fr) {return fr.frameSize;};
    uint16_t GetHeaderSize(GETFrame& fr) {return fr.headerSize;};
    uint16_t GetItemSize(GETFrame& fr) {return fr.itemSize;};
    uint32_t GetNItems(GETFrame& fr) {return fr.nItems;};
    uint32_t GetEventTime(GETFrame& fr) {return fr.eventTime;};
    uint32_t GetEventId(GETFrame& fr) {return fr.eventId;};
    uint8_t  GetCoboId(GETFrame& fr) {return fr.coboId;};
    uint8_t  GetAsadId(GETFrame& fr) {return fr.asadId;};
    
    unsigned long GetSizeOfDataVec(GETFrame& fr) {return fr.data.size();};
    
    std::vector<uint8_t> fakeData;
    MockGetDataFile mockFile;
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

TEST_F(GETFrameTestFixture, Constructor)
{
    EXPECT_CALL(mockFile, GetNextRawFrame()).Times(1).WillOnce(testing::Return(fakeData));
    EXPECT_CALL(mockFile, GetFileCobo()).Times(1).WillOnce(testing::Return(0));
    EXPECT_CALL(mockFile, GetFileAsad()).Times(1).WillOnce(testing::Return(0));
    
    GETFrame frame {mockFile};
    EXPECT_EQ(6,GetMetaType(frame));
    EXPECT_EQ(1224,GetFrameSize(frame));
    EXPECT_EQ(4,GetHeaderSize(frame));
    EXPECT_EQ(4, GetItemSize(frame));
    EXPECT_EQ(19520, GetNItems(frame));
    EXPECT_EQ(1234567890, GetEventTime(frame));
    EXPECT_EQ(12, GetEventId(frame));
    EXPECT_EQ(0, GetCoboId(frame));
    EXPECT_EQ(0, GetAsadId(frame));
    EXPECT_EQ(19520, GetSizeOfDataVec(frame));
}

