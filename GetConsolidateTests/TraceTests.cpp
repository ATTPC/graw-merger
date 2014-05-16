//
//  TraceTests.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/16/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "Trace.h"

#include <map>

#include "gtest/gtest.h"

TEST(TraceTests,CompactSample)
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

TEST(TraceTests,UnpackSample)
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

TEST(TraceTests,CompactAndUnpack)
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

TEST(TraceTests,Addition) {
    Trace *t1, *t2;
    
    for (int a = 0; a < 200; a++) {
        for (int b = 0; b < 200; b++) {
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

TEST(TraceTests,Subtraction)
{
    Trace *t1, *t2;
    
    for (int a = 0; a < 200; a++) {
        for (int b = 0; b < 200; b++) {
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

TEST(TraceTests,ScalarDivision)
{
    Trace *t1;
    
    for (int a = 0; a < 200; a++) {
        for (int b = 1; b < 200; b++) {
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