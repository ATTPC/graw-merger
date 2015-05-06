//
//  main.cpp
//  get-manipTests
//
//  Created by Joshua Bradt on 5/16/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include <iostream>
#include "gtest/gtest.h"

int main(int argc, char ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

