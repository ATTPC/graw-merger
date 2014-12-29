//
//  EventTests.cpp
//  get-manip
//
//  Created by Joshua Bradt on 5/21/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include <iostream>

#include "gtest/gtest.h"
#include "Event.h"
#include "FakeRawFrame.h"
#include "GRAWFrame.h"
#include "LookupTable.h"
#include "Constants.h"

class EventTestFixture : public testing::Test
{
public:
    void TestSubtractPedestals();
    void TestApplyThreshold();
};

void EventTestFixture::TestSubtractPedestals()
{
    LookupTable<sample_t> pedsTable {};
    const sample_t pedsValue = 100;
    const sample_t dataValue = 200;
    
    Event evt {};
    
    for (addr_t cobo = 0; cobo < Constants::num_cobos; cobo++) {
        for (addr_t asad = 0; asad < Constants::num_asads; asad++) {
            for (addr_t aget = 0; aget < Constants::num_agets; aget++) {
                for (addr_t ch = 0; ch < Constants::num_channels; ch++) {
                    auto pedHash = pedsTable.CalculateHash(cobo, asad, aget, ch);
                    pedsTable.table.emplace(pedHash, pedsValue);
                    
                    Trace tr {};
                    for (tb_t tb = 0; tb < Constants::num_tbs; tb++) {
                        tr.data.emplace(tb, dataValue);
                    }
                    auto trHash = evt.CalculateHash(cobo, asad, aget, ch);
                    evt.traces.emplace(trHash, tr);
                }
            }
        }
    }
    
    evt.SubtractPedestals(pedsTable);
    
    for (const auto& trace : evt.traces) {
        for (tb_t tb = 0; tb < Constants::num_tbs; tb++) {
            auto result = trace.second.GetSample(tb);
            ASSERT_EQ(dataValue - pedsValue, result);
        }
    }
}

TEST_F(EventTestFixture, TestSubtractPedestals)
{
    TestSubtractPedestals();
}

void EventTestFixture::TestApplyThreshold()
{
    const sample_t threshold = 500;
    
    Event evt {};
    
    for (sample_t cobo = 0; cobo < Constants::num_cobos; cobo++) {
        for (sample_t asad = 0; asad < Constants::num_asads; asad++) {
            for (sample_t aget = 0; aget < Constants::num_agets; aget++) {
                for (sample_t ch = 0; ch < Constants::num_channels; ch++) {
                    Trace tr {};
                    
                    tr.coboId = cobo;
                    tr.asadId = asad;
                    tr.agetId = aget;
                    tr.channel = ch;
                    
                    for (sample_t tb = 0; tb < Constants::num_tbs; tb++) {
                        sample_t dataValue = (cobo+asad+aget+ch+tb)*5;
                        tr.AppendSample(tb, dataValue);
                    }
                    auto trHash = evt.CalculateHash(cobo, asad, aget, ch);
                    evt.traces.emplace(trHash, tr);
                }
            }
        }
    }
    
    evt.ApplyThreshold(threshold);
    
    for (const auto& trace : evt.traces) {
        sample_t cobo = trace.second.coboId;
        sample_t asad = trace.second.asadId;
        sample_t aget = trace.second.agetId;
        sample_t ch   = trace.second.channel;
        for (sample_t tb = 0; tb < Constants::num_tbs; tb++) {
            auto result = trace.second.GetSample(tb);
            sample_t expected = (cobo+asad+aget+ch+tb)*5;
            if (expected < threshold) {
                expected = 0;
            }
            ASSERT_EQ(expected, result);
        }
    }
}

TEST_F(EventTestFixture, TestApplyThreshold)
{
    TestApplyThreshold();
}