//
//  DataFile.h
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/19/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __GETConsolidate__DataFile__
#define __GETConsolidate__DataFile__

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <string>
#include <map>
#include <vector>

#include "GETExceptions.h"

class DataFile
{
public:
    DataFile();
    DataFile(const std::string& path, const std::ios::openmode mode);
    DataFile(const boost::filesystem::path& path, const std::ios::openmode mode);
    
//    DataFile(const DataFile& orig) =delete;
//    DataFile(DataFile&& orig) =delete;
//    
//    DataFile& operator=(const DataFile& other) =delete;
//    DataFile& operator=(DataFile&& other) =delete;
    
    virtual void OpenFileForWrite(const std::string& path);
    virtual void OpenFileForWrite(const boost::filesystem::path& path);
    
    virtual void OpenFileForRead(const std::string& path);
    virtual void OpenFileForRead(const boost::filesystem::path& path);
    
    virtual void CloseFile();
    
    virtual std::vector<uint8_t> ReadRawFrame() = 0;
    
    virtual bool eof() const;
    
protected:
    boost::filesystem::path filePath;
    std::fstream filestream;
    
    bool isInitialized = false;
    bool isEOF = false;
};

#endif /* defined(__GETConsolidate__DataFile__) */
