//
//  PadLookupTable.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "PadLookupTable.h"
#include "GETExceptions.h"
#include <string>
#include <sstream>

uint32_t PadLookupTable::CalculateHash(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel)
{
    return channel + aget*100 + asad*10000 + cobo*1000000;
}

uint16_t PadLookupTable::FindPadNumber(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel)
{
    if (!table.empty()) {
        uint16_t hash = CalculateHash(cobo, asad, aget, channel);
        auto foundItem = table.find(hash);
        if (foundItem != table.end()) {
            return foundItem->second;
        }
        else {
            return 0;
        }
    }
    else {
        throw Exceptions::Not_Init();
    }
}

PadLookupTable::PadLookupTable(const std::string& path)
{
    std::ifstream file (path, std::ios::in|std::ios::binary);
    
    // MUST throw out the first two junk lines in file. No headers!
    
    if (!file.good()) throw 0; // FIX THIS!
        
    std::string line;
        
    while (!file.eof()) {
        int cobo, asad, aget, channel;
        int padNumber;
        getline(file,line,'\n'); // PROBLEM: Igor outputs \r
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