//
//  PadLookupTable.cpp
//  get-manip
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "PadLookupTable.h"
#include "GMExceptions.h"
#include <string>
#include <sstream>

uint32_t PadLookupTable::CalculateHash(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel)
{
    // Widen the integers so they don't overflow on multiplication
    auto wcobo = uint32_t(cobo);
    auto wasad = uint32_t(asad);
    auto waget = uint32_t(aget);
    auto wchannel = uint32_t(channel);
    
    auto result = wchannel + waget*100 + wasad*10000 + wcobo*1000000;
    
    return result;
}

uint16_t PadLookupTable::FindPadNumber(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel)
{
    if (!table.empty()) {
        auto hash = CalculateHash(cobo, asad, aget, channel);
        auto foundItem = table.find(hash);
        if (foundItem != table.end()) {
            return foundItem->second;
        }
        else {
            return missingPad; // an invalid value
        }
    }
    else {
        throw Exceptions::Not_Init();
    }
}

PadLookupTable::PadLookupTable()
{}

PadLookupTable::PadLookupTable(const std::string& path)
{
    std::ifstream file (path, std::ios::in|std::ios::binary);
    
    // MUST throw out the first two junk lines in file. No headers!
    
    if (!file.good()) throw 0; // FIX THIS!
        
    std::string line;
        
    while (!file.eof()) {
        int cobo, asad, aget, channel;
        int padNumber;
        getline(file,line,'\r'); // PROBLEM: Igor outputs \r
        std::stringstream lineStream(line);
        std::string element;
        
        getline(lineStream, element,',');
        if (element == "-1" || element == "") continue; // KLUDGE!
        cobo = stoi(element);
        
        getline(lineStream, element,',');
        asad = stoi(element);
        
        getline(lineStream, element,',');
        aget = stoi(element);
        
        getline(lineStream, element,',');
        channel = stoi(element);
        
        uint32_t hash = CalculateHash(cobo, asad, aget, channel);
        
        getline(lineStream, element);
        padNumber = stoi(element);
        
        table.emplace(hash, padNumber);
    }
}