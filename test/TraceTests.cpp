//
//  TraceTests.cpp
//  get-manip
//
//  Created by Joshua Bradt on 5/16/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "Trace.h"
#include "Constants.h"

#include <map>
#include <algorithm>
#include <sstream>
#include <vector>

#include "gtest/gtest.h"

class TraceTestFixture : public testing::Test
{
public:
    void TestCompactSample();
    void TestCompactSampleWithNegative();
    void TestUnpackSample();
    void TestUnpackSampleWithNegative();
    void TestCompactAndUnpack();
    void TestCompactAndUnpackWithNegative();
    void TestAddition();
    void TestSubtraction();
    void TestScalarDivision();
    void TestScalarSubtraction();
    void TestNormalization(Trace& tr);
    void TestEquality(Trace& tr1, Trace& tr2);
    void TestThreshold();
    void TestDropZeros();
    
    void TestCopyAssignment();
    void TestMoveAssignment();
    void TestCopyConstructor();
    void TestMoveConstructor();
};

void TraceTestFixture::TestEquality(Trace &tr1, Trace &tr2)
{
    EXPECT_EQ(tr1.coboId, tr2.coboId);
    EXPECT_EQ(tr1.asadId, tr2.asadId);
    EXPECT_EQ(tr1.agetId, tr2.agetId);
    EXPECT_EQ(tr1.channel, tr2.channel);
    EXPECT_EQ(tr1.padId, tr2.padId);
    
    for (int i = 0; i < 512; i++) {
        EXPECT_EQ(tr1.GetSample(i), tr2.GetSample(i));
    }
}

TEST_F(TraceTestFixture, TestCopy_DestinationEmpty)
{
    Trace tr1 {3,2,1,23,4563};
    for (int i = 0; i < 512; i++) {
        tr1.AppendSample(i, 512);
    }
    
    Trace tr2 {};
    
    tr2 = tr1;
    
    TestEquality(tr1, tr2);
}

TEST_F(TraceTestFixture, TestCopy_DestinationFull)
{
    Trace tr1 {3,2,1,23,4563};
    Trace tr2 {4,1,0,45,123};
    for (int i = 0; i < 512; i++) {
        tr1.AppendSample(i, 512);
        tr2.AppendSample(i, 200);
    }
    
    tr2 = tr1;
    
    TestEquality(tr1, tr2);
}

void TraceTestFixture::TestCompactSample()
{
    for (uint16_t tb = 0; tb < 512; tb++) {
        for (int16_t val = 0; val < 0xFFF; val++) {
            uint32_t res = Trace::CompactSample(tb, val);
            uint16_t res_tb = (res & 0xFF8000) >> 15;
            int16_t res_val = res & 0xFFF;
            ASSERT_EQ(tb, res_tb);
            ASSERT_EQ(val, res_val);
        }
    }
}

TEST_F(TraceTestFixture, TestCompactSample)
{
    TestCompactSample();
}

void TraceTestFixture::TestCompactSampleWithNegative()
{
    for (uint16_t tb = 0; tb < 512; tb++) {
        for (int16_t val = -4095; val < 0xFFF; val++) {
            uint32_t res = Trace::CompactSample(tb, val);
            uint16_t res_tb = (res & 0xFF8000) >> 15;
            int16_t res_val = res & 0xFFF;
            int16_t res_par = (res & 0x1000) >> 12;
            if (res_par == 1) {
                res_val *= -1;
            }
            ASSERT_EQ(tb, res_tb);
            ASSERT_EQ(val, res_val);
        }
    }
}

TEST_F(TraceTestFixture, TestCompactSampleWithNegative)
{
    TestCompactSampleWithNegative();
}

void TraceTestFixture::TestUnpackSample()
{
    for (uint16_t tb = 0; tb < 512; tb++) {
        for (int16_t val = 0; val < 0xFFF; val++) {
            uint32_t compacted = (tb << 15) | val;
            auto unpacked = Trace::UnpackSample(compacted);
            ASSERT_EQ(tb, unpacked.first);
            ASSERT_EQ(val, unpacked.second);
        }
    }
}

TEST_F(TraceTestFixture, TestUnpackSample)
{
    TestUnpackSample();
}

void TraceTestFixture::TestUnpackSampleWithNegative()
{
    for (uint16_t tb = 0; tb < 512; tb++) {
        for (int16_t val = -4095; val < 0xFFF; val++) {
            int16_t parity = val < 0 ? (1 << 12) : 0;
            uint32_t compacted = (tb << 15) | abs(val) | parity;
            auto unpacked = Trace::UnpackSample(compacted);
            ASSERT_EQ(tb, unpacked.first);
            ASSERT_EQ(val, unpacked.second);
        }
    }
}

TEST_F(TraceTestFixture, TestUnpackSampleWithNegative)
{
    TestUnpackSampleWithNegative();
}

void TraceTestFixture::TestCompactAndUnpack()
{
    for (uint16_t tb = 0; tb < 512; tb++) {
        for (int16_t val = 0; val < 0xFFF; val++) {
            uint32_t comp = Trace::CompactSample(tb, val);
            auto unp = Trace::UnpackSample(comp);
            ASSERT_EQ(tb, unp.first);
            ASSERT_EQ(val, unp.second);
        }
    }
}

TEST_F(TraceTestFixture,TestCompactAndUnpack)
{
    TestCompactAndUnpack();
}

void TraceTestFixture::TestCompactAndUnpackWithNegative()
{
    for (uint16_t tb = 0; tb < 512; tb++) {
        for (int16_t val = -4095; val < 0xFFF; val++) {
            uint32_t comp = Trace::CompactSample(tb, val);
            auto unp = Trace::UnpackSample(comp);
            ASSERT_EQ(tb, unp.first);
            ASSERT_EQ(val, unp.second);
        }
    }
}

TEST_F(TraceTestFixture,TestCompactAndUnpackWithNegative)
{
    TestCompactAndUnpackWithNegative();
}

void TraceTestFixture::TestAddition()
{
    Trace *t1, *t2;
    
    for (int a = 200; a < 210; a++) {
        for (int b = 200; b < 210; b++) {
            t1 = new Trace ();
            t2 = new Trace ();
            for (int i = 0; i < 512; i++) {
                t1->AppendSample(i, a);
                t2->AppendSample(i, b);
            }
            *t1 += *t2;
            
            for (int i = 0; i < 512; i++) {
                int16_t val = t1->GetSample(i);
                ASSERT_EQ(a+b, val);
            }
            delete t1;
            delete t2;
        }
    }
}

TEST_F(TraceTestFixture,TestAddition)
{
    TestAddition();
}

void TraceTestFixture::TestSubtraction()
{
    Trace *t1, *t2;
    
    for (int a = 200; a < 210; a++) {
        for (int b = 200; b < 210; b++) {
            t1 = new Trace ();
            t2 = new Trace ();
            for (int i = 0; i < 512; i++) {
                t1->AppendSample(i, a);
                t2->AppendSample(i, b);
            }
            *t1 -= *t2;
            
            for (int i = 0; i < 512; i++) {
                int16_t val = t1->GetSample(i);
                ASSERT_EQ(a-b, val);
            }
            delete t1;
            delete t2;
        }
    }
}

TEST_F(TraceTestFixture,TestSubtraction)
{
    TestSubtraction();
}

void TraceTestFixture::TestScalarDivision()
{
    Trace *t1;
    
    for (int a = 200; a < 210; a++) {
        for (int b = 1; b < 10; b++) {
            t1 = new Trace ();
            for (int i = 0; i < 512; i++) {
                t1->AppendSample(i, a);
            }
            *t1 /= b;
            
            for (int i = 0; i < 512; i++) {
                int16_t val = t1->GetSample(i);
                ASSERT_EQ(a/b, val);
            }
            delete t1;
        }
    }
}

TEST_F(TraceTestFixture, TestScalarDivision)
{
    TestScalarDivision();
}

void TraceTestFixture::TestScalarSubtraction()
{
    Trace *t1;
    
    for (sample_t a = 200; a < 210; a++) {
        for (sample_t b = 1; b < 10; b++) {
            t1 = new Trace ();
            for (int i = 0; i < 512; i++) {
                t1->AppendSample(i, a);
            }
            *t1 -= b;
            
            for (int i = 0; i < 512; i++) {
                auto val = t1->GetSample(i);
                ASSERT_EQ(a-b, val);
            }
            delete t1;
        }
    }
}

TEST_F(TraceTestFixture, TestScalarSubtraction)
{
    TestScalarSubtraction();
}

void TraceTestFixture::TestNormalization(Trace& tr)
{
    tr.RenormalizeToZero();
    
    int mean = 0;
    for (const auto& item : tr.data) {
        mean += item.second;
    }
    mean /= tr.GetNumberOfTimeBuckets();
    
    ASSERT_EQ(mean, 0);
}

TEST_F(TraceTestFixture, TestNormalization_linear)
{
    Trace tr {};
    for (int i = 0; i < 512; i++) {
        tr.AppendSample(i, i);
    }
    TestNormalization(tr);
}

TEST_F(TraceTestFixture, TestNormalization_linear_norm)
{
    Trace tr {};
    for (int i = 0; i < 512; i++) {
        tr.AppendSample(i, i - 256);
    }
    TestNormalization(tr);
}

TEST_F(TraceTestFixture, TestNormalization_quad)
{
    Trace tr {};
    for (int i = 0; i < 512; i++) {
        tr.AppendSample(i, (i/16)^2);
    }
    TestNormalization(tr);
}

TEST_F(TraceTestFixture, TestFlatFPN_NoChange)
{
    Trace data {};
    Trace FPN {};
    
    for (int i = 0; i < 512; i++) {
        FPN.AppendSample(i, 200);
        data.AppendSample(i, i/2 + 50);
    }
    
    FPN.RenormalizeToZero();
    
    Trace orig = data;
    data -= FPN;
    for (int i = 0; i < 512; i++) {
        EXPECT_EQ(orig.GetSample(i), data.GetSample(i));
        EXPECT_EQ(0, FPN.GetSample(i));
    }
}

TEST_F(TraceTestFixture, TestNormalization_WithNoData)
{
    Trace empty {};
    ASSERT_THROW(empty.RenormalizeToZero(), Exceptions::No_Data);
}

TEST_F(TraceTestFixture, TestSize)
{
    Trace tr {};
    for (int i = 0; i < 512; i++) {
        tr.AppendSample(i, 200);
    }
    std::stringstream ss;
    
    ss << tr;
    
    ASSERT_EQ(ss.tellp(), tr.size());
}

TEST_F(TraceTestFixture, DISABLED_TestSerializeAndUnpack)
{
    Trace tr1 {3,2,1,45,1234};
    
    for (int i = 0; i < 512; i++) {
        tr1.AppendSample(i, 400);
    }
    
    std::stringstream ss {std::ios::in|std::ios::out|std::ios::binary};
    
    int temp {0x0a0a0a0a};
    ss.write((char*) &temp, sizeof(temp));
    
    ss << tr1;
    ss.seekg(0);
    
    std::vector<uint8_t> raw;
    
    for (uint8_t ch; ss >> ch; ) {
        raw.push_back(ch);
    }
    
    Trace tr2 {raw};
    
    TestEquality(tr1, tr2);
}

void TraceTestFixture::TestThreshold()
{
    Trace tr {};
    for (tb_t tb = 0; tb < Constants::num_tbs; tb++) {
        tr.AppendSample(tb, tb*2); // These will range [0, 1024)
    }
    
    sample_t threshold = 200;
    
    tr.ApplyThreshold(threshold);
    
    for (tb_t tb = 0; tb < Constants::num_tbs; tb++) {
        sample_t expected = tb*2 < threshold ? 0 : tb*2;
        auto result = tr.GetSample(tb);
        ASSERT_EQ(expected, result);
    }
}

TEST_F(TraceTestFixture, TestThreshold)
{
    TestThreshold();
}

void TraceTestFixture::TestDropZeros()
{
    Trace tr {};
    for (tb_t tb = 0; tb < Constants::num_tbs; tb++) {
        tr.AppendSample(tb, tb % 2);
    }
    tr.DropZeros();
    
    for (tb_t tb = 0; tb < Constants::num_tbs; tb++) {
        if (tb % 2) {
            ASSERT_NO_THROW(tr.GetSample(tb));
        }
        else {
            ASSERT_THROW(tr.GetSample(tb), std::out_of_range);
        }
        
    }
}

TEST_F(TraceTestFixture, TestDropZeros)
{
    TestDropZeros();
}

void TraceTestFixture::TestCopyAssignment()
{
    addr_t cobo = 4;
    addr_t asad = 2;
    addr_t aget = 1;
    addr_t channel = 14;
    pad_t pad = 1942;
    
    Trace tr {cobo, asad, aget, channel, pad};
    
    for (tb_t tb = 0; tb < Constants::num_tbs; tb++) {
        tr.data[tb] = tb*2;
    }
    
    Trace tr2 = tr;
    
    TestEquality(tr, tr2);
    
    ASSERT_NE(&(tr.data), &(tr2.data));
}

TEST_F(TraceTestFixture, TestCopyAssignment)
{
    TestCopyAssignment();
}

void TraceTestFixture::TestMoveAssignment()
{
    addr_t cobo = 4;
    addr_t asad = 2;
    addr_t aget = 1;
    addr_t channel = 14;
    pad_t pad = 1942;
    
    Trace tr {cobo, asad, aget, channel, pad};
    
    for (tb_t tb = 0; tb < Constants::num_tbs; tb++) {
        tr.data[tb] = tb*2;
    }
    
    Trace tr2 = std::move(tr);
    
    for (tb_t tb = 0; tb < Constants::num_tbs; tb++) {
        ASSERT_EQ(tb*2, tr2.data[tb]);
    }
    
    ASSERT_EQ(0, tr.data.size());
}

TEST_F(TraceTestFixture, TestMoveAssignment)
{
    TestMoveAssignment();
}

void TraceTestFixture::TestCopyConstructor()
{
    addr_t cobo = 4;
    addr_t asad = 2;
    addr_t aget = 1;
    addr_t channel = 14;
    pad_t pad = 1942;
    
    Trace tr {cobo, asad, aget, channel, pad};
    
    for (tb_t tb = 0; tb < Constants::num_tbs; tb++) {
        tr.data[tb] = tb*2;
    }
    
    Trace tr2 {tr};
    
    TestEquality(tr, tr2);
    
    ASSERT_NE(&(tr.data), &(tr2.data));
}

TEST_F(TraceTestFixture, TestCopyConstructor)
{
    TestCopyConstructor();
}

void TraceTestFixture::TestMoveConstructor()
{
    addr_t cobo = 4;
    addr_t asad = 2;
    addr_t aget = 1;
    addr_t channel = 14;
    pad_t pad = 1942;
    
    Trace tr {cobo, asad, aget, channel, pad};
    
    for (tb_t tb = 0; tb < Constants::num_tbs; tb++) {
        tr.data[tb] = tb*2;
    }
    
    Trace tr2 {std::move(tr)};
    
    for (tb_t tb = 0; tb < Constants::num_tbs; tb++) {
        ASSERT_EQ(tb*2, tr2.data[tb]);
    }
    
    ASSERT_EQ(0, tr.data.size());
}

TEST_F(TraceTestFixture, TestMoveConstructor)
{
    TestMoveConstructor();
}