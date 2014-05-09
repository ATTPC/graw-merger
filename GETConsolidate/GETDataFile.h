//
//  GETDataFile.h
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/9/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __GETConsolidate__GETDataFile__
#define __GETConsolidate__GETDataFile__

#include <iostream>
#include <fstream>
#include <vector>
#include <boost/filesystem.hpp>
#include <exception>
#include <string>

#include "GETFrame.h"

class GETDataFile
{
private:
    boost::filesystem::path filePath;
    std::ifstream* filestream;
    
    uint8_t coboId;
    uint8_t asadId;
    
    template<typename outType>
    outType ExtractByteSwappedInt(std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end);

public:
    GETDataFile(const boost::filesystem::path& filePath_in);
    ~GETDataFile();
    
    std::vector<uint8_t>* GetNextRawFrame();
    
    uint8_t GetFileCobo ();
    uint8_t GetFileAsad ();
};

#endif /* defined(__GETConsolidate__GETDataFile__) */
