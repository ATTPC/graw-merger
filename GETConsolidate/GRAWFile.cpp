//
//  GRAWFile.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/9/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "GRAWFile.h"

// --------
// Constructors
// --------

GRAWFile::GRAWFile() {};

GRAWFile::GRAWFile(const boost::filesystem::path& path, const std::ios::openmode mode)
{
    if (mode & std::ios::in) {
        OpenFileForRead(path);
    }
    else if (mode & std::ios::out) {
        OpenFileForWrite(path);
    }
    else throw Exceptions::File_Open_Failed(path.string());
}

GRAWFile::GRAWFile(const std::string& path, const std::ios::openmode mode)
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

void GRAWFile::OpenFileForRead(const boost::filesystem::path& filePath_in)
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

void GRAWFile::OpenFileForRead(const std::string& path)
{
    boost::filesystem::path fp {path};
    OpenFileForRead(fp);
}

void GRAWFile::OpenFileForWrite(const boost::filesystem::path& path)
{
    DataFile::OpenFileForWrite(path);
}

void GRAWFile::OpenFileForWrite(const std::string& path)
{
    boost::filesystem::path fp {path};
    OpenFileForWrite(fp);
}

// --------
// Getters for Raw Data and Properties
// --------

std::vector<uint8_t> GRAWFile::ReadRawFrame()
{
    std::vector<uint8_t> size_raw;
    uint16_t size;
    std::vector<uint8_t> frame_raw;
    
    // Test the input file for validity
    
    if (!filestream.good()) {
        throw Exceptions::Bad_File(filePath.filename().string());
    }
    
    unsigned long storedPos = filestream.tellg();
    
    uint8_t metaType = filestream.get();
    
    for (int i = 0; i < 3; i++) {
        char temp;
        filestream.read(&temp, sizeof(uint8_t));
        size_raw.push_back((uint8_t)temp);
    }
    
    size = Utilities::ExtractByteSwappedInt<uint32_t>(size_raw.begin(), size_raw.end());
    
    if (size == 0) {
        if (filestream.eof()) {
            isEOF = true;
            throw Exceptions::End_of_File();
        }
        else {
            throw Exceptions::Frame_Read_Error();
        }
    }
    
    // See if this size is right by checking if we get to the next frame
    
    filestream.seekg(storedPos + size*GRAWFrame::sizeUnit);
    
    if (filestream.peek() != metaType) {
        if (filestream.eof()) {
            // This is the last frame in the file.
            isEOF = true;
        }
        else {
            // This isn't the last frame. The size must be wrong.
            filestream.seekg(12, std::ios::cur);
            
            std::vector<uint8_t> nItems_raw;
            for (int i = 0; i < 4; i++) {
                char temp;
                filestream.read(&temp, sizeof(uint8_t));
                nItems_raw.push_back((uint8_t)temp);
            }
            
            uint32_t nItems = Utilities::ExtractByteSwappedInt<uint32_t>(nItems_raw.begin(), nItems_raw.end());
            
            size = ceil(double(GRAWFrame::Expected_headerSize*GRAWFrame::sizeUnit + GRAWFrame::Expected_itemSize * nItems)/GRAWFrame::sizeUnit);
            
            filestream.seekg(storedPos + size*GRAWFrame::sizeUnit);
            
            if (filestream.peek() != metaType) {
                // Give up
                throw Exceptions::Frame_Read_Error();
            }
        }
    }
    
    filestream.seekg(storedPos); // rewind to start of frame
    
    for (unsigned long i = 0; i < size*GRAWFrame::sizeUnit; i++) {
        char temp;
        filestream.read(&temp, sizeof(uint8_t));
        frame_raw.push_back((uint8_t)temp);
    }
    
    return frame_raw;
    // Leaves file pointer at end of frame. This assumes the frame size is correct.
}

void GRAWFile::WriteFrame(const GRAWFrame& frame)
{
    // Serialize the frame. **Big-endian**
    std::vector<uint8_t> ser;
    
    ser.push_back(frame.metaType);
    AppendBytes(ser, frame.frameSize, 3);
    ser.push_back(frame.dataSource);
    AppendBytes(ser, frame.frameType, 2);
    ser.push_back(frame.revision);
    AppendBytes(ser, frame.headerSize, 2);
    AppendBytes(ser, frame.itemSize, 2);
    AppendBytes(ser, frame.nItems, 4);
    AppendBytes(ser, frame.eventTime, 6);
    AppendBytes(ser, frame.eventId, 4);
    ser.push_back(frame.coboId);
    ser.push_back(frame.asadId);
    AppendBytes(ser, frame.readOffset, 2);
    ser.push_back(frame.status);
    
//    for (auto item : frame.hitPatterns) {
//        AppendBytes(ser, item.to_ullong(), 9);
//    }
    
    for (int i = 0; i < 9*4; i++) {
        ser.push_back(0xFF);
    }
    
    for (auto item : frame.multiplicity) {
        AppendBytes(ser, item, 2);
    }
    
    // Pad it out
    
    while (ser.size() < frame.headerSize * GRAWFrame::sizeUnit) {
        ser.push_back(0x00);
    }
    
    for (auto item : frame.data) {
        uint32_t ser_item {0};
        ser_item |= item.GetSample();
        ser_item |= (item.GetTimeBucketId() << 14);
        ser_item |= (item.GetChannel() << 23);
        ser_item |= (item.GetAgetId() << 30);
        AppendBytes(ser, ser_item, 4);
    }
    
    while (ser.size() < frame.frameSize * GRAWFrame::sizeUnit) {
        ser.push_back(0x00);
    }
    
    // Write to file
    for (auto item : ser) {
        filestream.write((char*) &item,sizeof(item));
    }
}

template<typename T>
void GRAWFile::AppendBytes(std::vector<uint8_t>& vec, T val, int nBytes)
{
    for (int i = nBytes-1; i >= 0; i--) {
        vec.push_back((val & (0xFF << i*8)) >> i*8);
    }
}

uint8_t GRAWFile::GetFileCobo() const
{
    return coboId;
}

uint8_t GRAWFile::GetFileAsad() const
{
    return asadId;
}