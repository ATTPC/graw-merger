//
//  EventFile.cpp
//  get-manip
//
//  Created by Joshua Bradt on 5/9/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "EventFile.h"

// --------
// Static constants
// --------

const int EventFile::magic {0x6e7ef11e};

// --------
// Opening and Closing the File
// --------

void EventFile::OpenFileForWrite(const boost::filesystem::path& path)
{
    DataFile::OpenFileForWrite(path);
    
    filestream.write((char*) &(EventFile::magic), sizeof(EventFile::magic));
}

void EventFile::OpenFileForWrite(const std::string& path)
{
    boost::filesystem::path fp {path};
    OpenFileForWrite(fp);
}

void EventFile::OpenFileForRead(const boost::filesystem::path& path)
{
    DataFile::OpenFileForRead(path);
    
    int read_magic;
    filestream.read((char*) &read_magic,sizeof(read_magic));
    if (read_magic != magic) throw Exceptions::Wrong_File_Type(path.string());
    
    // This should leave the file position at the start of the first event.
}

void EventFile::OpenFileForRead(const std::string& path)
{
    boost::filesystem::path fp {path};
    OpenFileForRead(fp);
}

// --------
// Functions for Writing Events to File
// --------

void EventFile::WriteEvent(const Event& event)
{
    // Make sure file is initialized
    
    if (!isInitialized) {
        throw Exceptions::Not_Init();
    }
    
    unsigned long long currentPos = filestream.tellg();
    offsetTable.emplace(event.eventId, currentPos);
    
    filestream << event;
}

// --------
// Functions for Reading Events from File
// --------

void EventFile::RebuildIndex()
{
    if (!isInitialized) throw Exceptions::Not_Init();
    
    std::cout << "Rebuilding file's event index." << std::endl;
    
    filestream.seekg(sizeof(EventFile::magic));  // go to end of file magic number
    
    while (!filestream.eof()) {
        unsigned long long currentPos = filestream.tellg();
        uint8_t magic_in;
        uint32_t eventSize, eventId;
        filestream.read((char*) &magic_in, sizeof(magic_in));
        
        if (magic_in != Event::magic) throw Exceptions::Wrong_File_Position();
        
        filestream.read((char*) &eventSize, sizeof(eventSize));
        filestream.read((char*) &eventId, sizeof(eventId));
        offsetTable.emplace(eventId,currentPos);
        // Go back by the size of the three things we read, and skip forward
        // by the size of the event.
        filestream.seekg(eventSize - sizeof(magic_in) - sizeof(eventSize) - sizeof(eventId),std::ios::cur);
    }
    currentEvt = offsetTable.begin();
    filestream.clear();
    filestream.seekg(currentEvt->second);
}

std::vector<uint8_t> EventFile::ReadRawFrame()
{
    if (!isInitialized) throw Exceptions::Not_Init();
    
    if (offsetTable.size() == 0) {
        RebuildIndex();
    }
    
    // Check that the file pointer is at the beginning of the event
    
    if (filestream.tellg() != currentEvt->second) {
        throw Exceptions::Wrong_File_Position();
    }
    
    // Read in the magic number and check it
    
    uint8_t read_magic {};
    
    filestream.read((char*) &read_magic, sizeof(read_magic));
    if (read_magic != Event::magic) throw Exceptions::Wrong_File_Position();
    
    // Find the event size
    
    uint32_t eventSize;
    
    filestream.read((char*) &eventSize, sizeof(eventSize));
    
    // Now go back to where we started and read the whole event
    
    std::vector<uint8_t> event_raw {};
    
    filestream.seekg(currentEvt->second);
    for (unsigned long i = 0; i < eventSize; i++) {
        uint8_t temp {};
        filestream.read((char*) &temp, sizeof(char));
        event_raw.push_back(temp);
    }
    
    // Increment the current event iterator
    
    currentEvt++;
    
    return event_raw;
}

Event EventFile::GetNextEvent()
{
    if (!isInitialized) throw Exceptions::Not_Init();
    
    if (offsetTable.size() == 0) RebuildIndex();
    
    if (currentEvt != offsetTable.end()) {
        std::vector<uint8_t> raw_event = ReadRawFrame();
        Event nextEvent {raw_event};
        return nextEvent;
    }
    else throw Exceptions::End_of_File();
}

Event EventFile::GetPreviousEvent()
{
    if (!isInitialized) throw Exceptions::Not_Init();
    
    if (currentEvt != offsetTable.begin()) {
        currentEvt--;
        std::vector<uint8_t> raw_event = ReadRawFrame();
        Event prevEvent {raw_event};
        return prevEvent;
    }
    else throw Exceptions::End_of_File();
}