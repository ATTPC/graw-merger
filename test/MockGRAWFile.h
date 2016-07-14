//
//  MockGRAWFile.h
//  get-manip
//
//  Created by Joshua Bradt on 5/16/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __get_manip__mock_GRAWFile_h
#define __get_manip__mock_GRAWFile_h

#include "gmock/gmock.h"

#include "GRAWFile.h"

class MockGRAWFile : public GRAWFile
{
public:
    MockGRAWFile() {};
    MOCK_METHOD0(GetNextRawFrame, std::vector<uint8_t>());
    MOCK_CONST_METHOD0(GetFileCobo, uint8_t());
    MOCK_CONST_METHOD0(GetFileAsad, uint8_t());
};

#endif
