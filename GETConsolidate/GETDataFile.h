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
    /* This class represents the .GRAW file output by the DAQ. It contains a 
     * C++ filestream to read in the file from disk, and it can then read
     * individual frames from the file and provide them to another object, such
     * as an instance of the GETFrame class.
     */
    
public:
    GETDataFile(const boost::filesystem::path& filePath_in);
    /* The constructor takes as an argument the path of the directory which
     * contains the .GRAW data files. It is assumed to be in the usual structure
     * where there is a folder called "mm[n]" for each CoBo, where the [n] is 
     * the CoBo number.
     */
    
    ~GETDataFile();
    
    /* Copy operators:
     * These are disabled since copying a filestream is a bad idea.
     */
    
    GETDataFile(const GETDataFile& orig) =delete;
    GETDataFile& operator=(const GETDataFile& other) =delete;
    
    // Move operators
    
    GETDataFile(GETDataFile&& orig);
    GETDataFile& operator=(GETDataFile&& orig);
    
    std::vector<uint8_t> GetNextRawFrame();
    /* This function returns the next raw frame from the data file as a vector
     * of integers. This can then be processed using, say, a GETFrame.
     */
    
    uint8_t GetFileCobo() const;    // Returns the CoBo # from the filename
    uint8_t GetFileAsad() const;    // Returns the AsAd # from the filename
    
    bool eof() const;               // Checks if the file pointer is at EOF
    
private:
    boost::filesystem::path filePath;    // The location of the file
    std::ifstream filestream;            // A filestream pointing to the file
    
    uint8_t coboId;                      // CoBo ID from the file name
    uint8_t asadId;                      // AsAd ID from the file name
    
    template<typename outType>
    outType ExtractByteSwappedInt(std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end);
    /* This function just byteswaps a provided integer. This is necessary since
     * the .GRAW files are generated on PowerPC and are big-endian, while Intel 
     * computers are little-endian.
     */
};

#endif /* defined(__GETConsolidate__GETDataFile__) */
