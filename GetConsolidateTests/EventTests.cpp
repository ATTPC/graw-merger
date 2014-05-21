//
//  EventTests.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/21/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include <iostream>

#include "gtest/gtest.h"
#include "Event.h"
#include "FakeRawFrame.h"
#include "GETFrame.h"

class EventTestFixture : public testing::Test
{
public:
    EventTestFixture()
    {
        FakeRawFrame fake_raw {1234567890, 12, 2, 3};
        fr = GETFrame {fake_raw.GenerateRawFrameVector(), 2, 3};
    }
    
    GETFrame fr;
    Event ev {};
};

