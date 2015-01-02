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
    virtual void SetUp();
    
    void TestEquality(const Event& e1, const Event& e2);
    
    void TestCopyConstructor();
    void TestCopyAssignment();
    
    void TestSubtractPedestals();
    void TestApplyThreshold();
    void TestDropZeros();
    
protected:
    Event evt;
    const sample_t dataValue = 200;
};

void EventTestFixture::SetUp()
{
    for (addr_t cobo = 0; cobo < Constants::num_cobos; cobo++) {
        for (addr_t asad = 0; asad < Constants::num_asads; asad++) {
            for (addr_t aget = 0; aget < Constants::num_agets; aget++) {
                for (addr_t ch = 0; ch < Constants::num_channels; ch++) {
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
}

void EventTestFixture::TestEquality(const Event& e1, const Event& e2)
{
    ASSERT_EQ(e2.eventId, e1.eventId);
    ASSERT_EQ(e2.eventTime, e1.eventTime);
    ASSERT_EQ(e2.nFramesAppended, e1.nFramesAppended);
    ASSERT_EQ(e2.lookupTable, e1.lookupTable);
    
    ASSERT_EQ(e2.traces.size(), e1.traces.size());
    
    for (const auto& item : e1.traces) {
        ASSERT_NO_THROW(e2.traces.at(item.first));
        
        const auto& tr1 = item.second;
        const auto& tr2 = e2.traces.at(item.first);
        
        ASSERT_EQ(tr2.size(), tr1.size());
        for (const auto& samp : tr1.data) {
            ASSERT_NO_THROW(tr2.data.at(samp.first));
            ASSERT_EQ(samp.second, tr2.data.at(samp.first));
        }
    }
}

void EventTestFixture::TestCopyConstructor()
{
    Event evt2 {evt};
    TestEquality(evt, evt2);
}

TEST_F(EventTestFixture, TestCopyConstructor)
{
    TestCopyConstructor();
}

void EventTestFixture::TestCopyAssignment()
{
    Event evt2 = evt;
    TestEquality(evt, evt2);
}

TEST_F(EventTestFixture, TestCopyAssignment)
{
    TestCopyAssignment();
}

void EventTestFixture::TestSubtractPedestals()
{
    LookupTable<sample_t> pedsTable {};
    const sample_t pedsValue = 100;
    
    for (addr_t cobo = 0; cobo < Constants::num_cobos; cobo++) {
        for (addr_t asad = 0; asad < Constants::num_asads; asad++) {
            for (addr_t aget = 0; aget < Constants::num_agets; aget++) {
                for (addr_t ch = 0; ch < Constants::num_channels; ch++) {
                    auto pedHash = pedsTable.CalculateHash(cobo, asad, aget, ch);
                    pedsTable.table.emplace(pedHash, pedsValue);
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
    
    Event evt_local {};
    
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
                    auto trHash = evt_local.CalculateHash(cobo, asad, aget, ch);
                    evt_local.traces.emplace(trHash, tr);
                }
            }
        }
    }
    
    evt_local.ApplyThreshold(threshold);
    
    for (const auto& trace : evt_local.traces) {
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

void EventTestFixture::TestDropZeros()
{
    Event evt_local {};
    
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
                        sample_t dataValue = 0;
                        if (ch % 2) {
                            dataValue = 0;
                        }
                        else {
                            dataValue = tb % 2;
                        }
                        tr.AppendSample(tb, dataValue);
                    }
                    auto trHash = evt_local.CalculateHash(cobo, asad, aget, ch);
                    evt_local.traces.emplace(trHash, tr);
                }
            }
        }
    }
    
    evt_local.DropZeros();
    
    for (sample_t cobo = 0; cobo < Constants::num_cobos; cobo++) {
        for (sample_t asad = 0; asad < Constants::num_asads; asad++) {
            for (sample_t aget = 0; aget < Constants::num_agets; aget++) {
                for (sample_t ch = 0; ch < Constants::num_channels; ch++) {
                    if (ch % 2) {
                        ASSERT_THROW(evt_local.GetTrace(cobo, asad, aget, ch),
                                     std::out_of_range);
                    }
                    else {
                        auto& tr = evt_local.GetTrace(cobo, asad, aget, ch);
                        for (sample_t tb = 0; tb < Constants::num_tbs; tb++) {
                            if (tb % 2) {
                                ASSERT_NO_THROW(tr.GetSample(tb));
                            }
                            else {
                                ASSERT_THROW(tr.GetSample(tb), std::out_of_range);
                            }
                        }
                    }
                }
            }
        }
    }
}

TEST_F(EventTestFixture, TestDropZeros)
{
    TestDropZeros();
}