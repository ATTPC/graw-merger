//
//  PadLookupTable.h
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __GETConsolidate__PadLookupTable__
#define __GETConsolidate__PadLookupTable__

#include <iostream>
#include <fstream>
#include <map>

class PadLookupTable
{
private:
    uint32_t CalculateHash(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel);
    
    std::map<uint32_t,uint16_t> table;
    
public:
    PadLookupTable(const std::string& path);
    
    uint16_t FindPadNumber(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel);
};

#endif /* defined(__GETConsolidate__PadLookupTable__) */
