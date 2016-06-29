//
//  LookupTable.cpp
//  get-manip
//
//  Created by Joshua Bradt on 12/27/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "LookupTable.h"
#include "GMExceptions.h"
#include <string>
#include <sstream>

template <typename mapped_t>
hash_t LookupTable<mapped_t>::CalculateHash(addr_t cobo, addr_t asad,
                                            addr_t aget, addr_t channel)
{
    // Widen the integers so they don't overflow on multiplication
    auto wcobo = hash_t(cobo);
    auto wasad = hash_t(asad);
    auto waget = hash_t(aget);
    auto wchannel = hash_t(channel);
    
    auto result = wchannel + waget*100 + wasad*10000 + wcobo*1000000;
    
    return result;
}

template <typename mapped_t>
mapped_t LookupTable<mapped_t>::Find(addr_t cobo, addr_t asad,
                                     addr_t aget, addr_t channel) const
{
    if (!table.empty()) {
        auto hash = CalculateHash(cobo, asad, aget, channel);
        auto foundItem = table.find(hash);
        if (foundItem != table.end()) {
            return foundItem->second;
        }
        else {
            return missingValue; // an invalid value
        }
    }
    else {
        throw Exceptions::Not_Init();
    }
}

template <typename mapped_t>
bool LookupTable<mapped_t>::Empty() const
{
    return table.empty();
}

template <typename mapped_t>
LookupTable<mapped_t>::LookupTable()
{}

template <typename mapped_t>
LookupTable<mapped_t>::LookupTable(const std::string& path)
{
    ReadFile(path);
}

template <typename mapped_t>
void LookupTable<mapped_t>::ReadFile(const std::string& path)
{
    std::ifstream file (path, std::ios::in|std::ios::binary);
    
    // MUST throw out the first two junk lines in file. No headers!
    
    if (!file.good()) throw 0; // FIX THIS!
    
    if (table.size() != 0) {
        table.clear();
    }
    
    std::string line;
    
    while (!file.eof()) {
        addr_t cobo, asad, aget, channel;
        mapped_t value;
        getline(file,line,'\n');
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
        
        auto hash = CalculateHash(cobo, asad, aget, channel);
        
        getline(lineStream, element);
        value = stoi(element);
        
        table.emplace(hash, value);
    }

}

// Explicit instantiations

template class LookupTable<pad_t>;
template class LookupTable<sample_t>;