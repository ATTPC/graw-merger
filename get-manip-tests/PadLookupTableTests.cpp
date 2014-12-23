//
//  PadLookupTableTests.cpp
//  get-manip
//
//  Created by Joshua Bradt on 12/23/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include <stdio.h>
#include "PadLookupTable.h"
#include "gtest/gtest.h"

class PadLookupTableTestFixture : public testing::Test
{
public:
    void TestCalculateHash();
};

void PadLookupTableTestFixture::TestCalculateHash()
{
    for (int cobo = 0; cobo < 10; cobo++) {
        for (int asad = 0; asad < 4; asad++) {
            for (int aget = 0; aget < 4; aget++) {
                for (int ch = 0; ch < 68; ch++) {
                    uint32_t trueHash = ch + 100*aget + 10000*asad + 1000000*cobo;
                    auto resHash = PadLookupTable::CalculateHash(cobo, asad, aget, ch);
                    ASSERT_EQ(trueHash, resHash);
                }
            }
        }
    }
}

TEST_F(PadLookupTableTestFixture, TestCalculateHash)
{
    TestCalculateHash();
}