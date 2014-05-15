//
//  EventFile.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/9/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "EventFile.h"

const char* EventFile::magic = "EVTFILE";

EventFile::EventFile ()
{
}

EventFile::~EventFile ()
{
    this->CloseFile();
}

void EventFile::CloseFile()
{
    if (file.is_open() and this->isInitialized) file.close();
}

void EventFile::OpenFileForWrite(std::string path)
{
    // Make sure the object isn't already initialized
    
    if (isInitialized) {
        throw Exceptions::Already_Init();
    }
    
    file.open(path, std::ios::out|std::ios::binary|std::ios::trunc);
    if (!file.good()) {
        throw Exceptions::Bad_File(path);
    }
    isInitialized = true;
    
    file.write(EventFile::magic,sizeof(EventFile::magic));
}

void EventFile::WriteEvent(const Event& event)
{
    // Make sure file is initialized
    
    if (!isInitialized) {
        throw Exceptions::Not_Init();
    }
    
    unsigned long long currentPos = file.tellg();
    offsetTable.emplace(event.eventId, currentPos);
    
    file << event;
}

void EventFile::OpenFileForRead(const std::string path)
{
    // Make sure the object isn't already initialized
    
    if (isInitialized) {
        throw Exceptions::Already_Init();
    }
    
    file.open(path, std::ios::in|std::ios::binary);
    if (!file.good()) {
        throw Exceptions::Bad_File(path);
    }
    isInitialized = true;
    
    // Check if this is the correct file format
    
    char read_magic[8];
    file >> read_magic;
    if (read_magic != magic) throw Exceptions::Wrong_File_Type(path);
    
    // This should leave the file position at the start of the first event.
}

Event EventFile::ReadEvent()
{
    if (!isInitialized) throw Exceptions::Not_Init();
    
    Event evt {};
    
    if (offsetTable.size() == 0) {
        RebuildIndex();
    }
    
    std::vector<char> event_raw {};
    
    // Check that the file pointer is at the beginning of the event
    
    if (file.tellg() != currentEvt->second) {
        throw Exceptions::Wrong_File_Position();
    }
    
    // Read in the magic number and check it
    
    char magic_in[4] {};
    uint32_t eventSize;
    file >> magic_in;
    
    if (magic_in != Event::magic) throw Exceptions::Wrong_File_Position();
    
    // Find the event size
    
    file >> eventSize;
    
    // Now go back to where we started and read the whole event
    
    file.seekg(currentEvt->second);
    for (unsigned long i = 0; i < eventSize; i++) {
        char temp {};
        file.read(&temp, sizeof(char));
        event_raw.push_back(temp);
    }
    
    // Increment the current event iterator
    
    currentEvt++;
    
    // Create the event
    
    Event res {event_raw};
    
    return res;
}

void EventFile::RebuildIndex()
{
    if (!isInitialized) throw Exceptions::Not_Init();
    
    std::cout << "Rebuilding file's event index." << std::endl;
    
    Event sampleEvt {};  // dummy event just to read values. KLUDGE
    
    file.seekg(sizeof(this->magic));  // go to end of file magic number
    
    while (!file.eof()) {
        unsigned long long currentPos = file.tellg();
        char magic_in[4] {};
        uint32_t eventSize, eventId;
        file >> magic_in;
        
        if (magic_in != sampleEvt.magic) throw Exceptions::Wrong_File_Position();
        
        file >> eventSize;
        file >> eventId;
        offsetTable.emplace(eventId,currentPos);
        // Go back by the size of the three things we read, and skip forward
        // by the size of the event.
        file.seekg(eventSize - sizeof(magic_in) - sizeof(eventSize) - sizeof(eventId));
    }
    
    file.seekg(sizeof(this->magic));
    currentEvt = offsetTable.begin();
}