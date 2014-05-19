//
//  GETDataFile.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/9/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "GETDataFile.h"

// --------
// Constructors
// --------

GETDataFile::GETDataFile() {};

GETDataFile::GETDataFile(const boost::filesystem::path path, const std::ios::openmode mode)
{
    if (mode & std::ios::in) {
        OpenFileForRead(path);
    }
    else if (mode & std::ios::out) {
        OpenFileForWrite(path);
    }
    else throw Exceptions::File_Open_Failed(path.string());
}

GETDataFile::GETDataFile(const std::string path, const std::ios::openmode mode)
{
    boost::filesystem::path fp {path};
    
    if (mode & std::ios::in) {
        OpenFileForRead(fp);
    }
    else if (mode & std::ios::out) {
        OpenFileForWrite(fp);
    }
    else throw Exceptions::File_Open_Failed(fp.string());
}

void GETDataFile::OpenFileForRead(const boost::filesystem::path filePath_in)
{
    DataFile::OpenFileForRead(filePath_in);
    
    if (filePath.extension() != ".graw") {
        throw Exceptions::Wrong_File_Type(filePath.filename().string());
    }
    
    // Extract info from path:
    // Expect path: /[data path]/timestamp/mm#/CoBo_AsAd#_[junk].graw
    
    std::string mmnStr = filePath.parent_path().string();
    
    coboId = std::stoi(&mmnStr.back()); // Last character
    
    std::string filename = filePath.filename().string();
    asadId = filename.at(filename.find("AsAd") + 4) - '0'; // this is kludgy
}

void GETDataFile::OpenFileForRead(const std::string path)
{
    boost::filesystem::path fp {path};
    OpenFileForRead(fp);
}

void GETDataFile::OpenFileForWrite(boost::filesystem::path path)
{
    
}

void GETDataFile::OpenFileForWrite(std::string path)
{
    boost::filesystem::path fp {path};
    OpenFileForWrite(fp);
}

// --------
// Getters for Raw Data and Properties
// --------

std::vector<uint8_t> GETDataFile::ReadRawFrame()
{
    std::vector<uint8_t> size_raw;
    uint16_t size;
    std::vector<uint8_t> frame_raw;
    
    // Test the input file for validity
    
    if (!filestream.good()) {
        throw Exceptions::Bad_File(filePath.filename().string());
    }
    
    filestream.seekg(1,std::ios::cur); // Skip the metaType char
    
    for (int i = 0; i < 3; i++) {
        char temp;
        filestream.read(&temp, sizeof(uint8_t));
        size_raw.push_back((uint8_t)temp);
    }
    
    size = Utilities::ExtractByteSwappedInt<uint32_t>(size_raw.begin(), size_raw.end());
    
    if (size == 0) throw Exceptions::Frame_Read_Error();
    
    std::cout << "Found frame of size " << size << std::endl;
    
    filestream.seekg(-4,std::ios::cur); // rewind to start of frame
    
    for (unsigned long i = 0; i < size*64; i++) {
        char temp;
        filestream.read(&temp, sizeof(uint8_t));
        frame_raw.push_back((uint8_t)temp);
    }
    
    return frame_raw;
    // Leaves file pointer at end of frame. This assumes the frame size is correct.
}

uint8_t GETDataFile::GetFileCobo() const
{
    return coboId;
}

uint8_t GETDataFile::GetFileAsad() const
{
    return asadId;
}