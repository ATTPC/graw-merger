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

class GETDataFile
{
private:
    boost::filesystem::path filePath;
    std::ifstream filestream;
    
    uint8_t coboId;
    uint8_t asadId;
    
    template<typename outType>
    outType ExtractByteSwappedInt(std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end);

public:
    GETDataFile(const boost::filesystem::path& filePath_in);
    ~GETDataFile();
    
    GETDataFile(const GETDataFile& orig) =delete;
    GETDataFile& operator=(const GETDataFile& other) =delete;
    
    GETDataFile(GETDataFile&& orig);
    GETDataFile& operator=(GETDataFile&& orig);
    
    std::vector<uint8_t> GetNextRawFrame();
    
    uint8_t GetFileCobo() const;
    uint8_t GetFileAsad() const;
    
    bool eof() const;
};

#endif /* defined(__GETConsolidate__GETDataFile__) */
