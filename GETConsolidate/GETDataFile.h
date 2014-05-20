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

#include "GETExceptions.h"
#include "Utilities.h"
#include "DataFile.h"
#include "GETFrame.h"

class GETDataFile : public DataFile
{
    /* This class represents the .GRAW file output by the DAQ. It contains a 
     * C++ filestream to read in the file from disk, and it can then read
     * individual frames from the file and provide them to another object, such
     * as an instance of the GETFrame class.
     */
    
public:
    
    GETDataFile();
    GETDataFile(const std::string& path, const std::ios::openmode mode);
    GETDataFile(const boost::filesystem::path& path, const std::ios::openmode mode);
    
    void OpenFileForRead(const std::string& path) override;
    void OpenFileForRead(const boost::filesystem::path& path) override;
    
    void OpenFileForWrite(const std::string& path) override;
    void OpenFileForWrite(const boost::filesystem::path& path) override;
    
    std::vector<uint8_t> ReadRawFrame() override;
    /* This function returns the next raw frame from the data file as a vector
     * of integers. This can then be processed using, say, a GETFrame.
     */
    
    void WriteFrame(const GETFrame& frame);
    
    virtual uint8_t GetFileCobo() const;    // Returns the CoBo # from the filename
    virtual uint8_t GetFileAsad() const;    // Returns the AsAd # from the filename
    
private:
    
    uint8_t coboId;                      // CoBo ID from the file name
    uint8_t asadId;                      // AsAd ID from the file name
    
    template<typename T>
    static void AppendBytes(std::vector<uint8_t>& vec, T val, int nBytes);
};

#endif /* defined(__GETConsolidate__GETDataFile__) */
