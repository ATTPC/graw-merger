//
//  EventFile.cpp
//  GETConsolidate
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
// Constructors and Destructor
// --------

EventFile::EventFile ()
{
}

EventFile::~EventFile ()
{
    this->CloseFile();
}

// --------
// Opening and Closing the File
// --------

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
    
    file.write((char*) &(EventFile::magic), sizeof(EventFile::magic));
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
    
    int read_magic;
    file.read((char*) &read_magic,sizeof(read_magic));
    if (read_magic != magic) throw Exceptions::Wrong_File_Type(path);
    
    // This should leave the file position at the start of the first event.
}

void EventFile::CloseFile()
{
    if (file.is_open() and this->isInitialized) file.close();
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
    
    unsigned long long currentPos = file.tellg();
    offsetTable.emplace(event.eventId, currentPos);
    
    file << event;
}

// --------
// Functions for Reading Events from File
// --------

void EventFile::RebuildIndex()
{
    if (!isInitialized) throw Exceptions::Not_Init();
    
    std::cout << "Rebuilding file's event index." << std::endl;
    
    file.seekg(sizeof(EventFile::magic));  // go to end of file magic number
    
    while (!file.eof()) {
        unsigned long long currentPos = file.tellg();
        uint8_t magic_in;
        uint32_t eventSize, eventId;
        file.read((char*) &magic_in, sizeof(magic_in));
        
        if (magic_in != Event::magic) throw Exceptions::Wrong_File_Position();
        
        file.read((char*) &eventSize, sizeof(eventSize));
        file.read((char*) &eventId, sizeof(eventId));
        offsetTable.emplace(eventId,currentPos);
        // Go back by the size of the three things we read, and skip forward
        // by the size of the event.
        file.seekg(eventSize - sizeof(magic_in) - sizeof(eventSize) - sizeof(eventId),std::ios::cur);
    }
    currentEvt = offsetTable.begin();
    file.clear();
    file.seekg(currentEvt->second);
}

Event EventFile::ReadEvent()
{
    if (!isInitialized) throw Exceptions::Not_Init();
    
    if (offsetTable.size() == 0) {
        RebuildIndex();
    }
    
    // Check that the file pointer is at the beginning of the event
    
    if (file.tellg() != currentEvt->second) {
        throw Exceptions::Wrong_File_Position();
    }
    
    // Read in the magic number and check it
    
    uint8_t read_magic {};
    
    file.read((char*) &read_magic, sizeof(read_magic));
    if (read_magic != Event::magic) throw Exceptions::Wrong_File_Position();
    
    // Find the event size
    
    uint32_t eventSize;
    
    file.read((char*) &eventSize, sizeof(eventSize));
    
    // Now go back to where we started and read the whole event
    
    std::vector<uint8_t> event_raw {};
    
    file.seekg(currentEvt->second);
    for (unsigned long i = 0; i < eventSize; i++) {
        uint8_t temp {};
        file.read((char*) &temp, sizeof(char));
        event_raw.push_back(temp);
    }
    
    // Increment the current event iterator
    
    currentEvt++;
    
    // Create the event
    
    Event res {event_raw};
    
    return res;
}

Event EventFile::GetNextEvent()
{
    if (!isInitialized) throw Exceptions::Not_Init();
    
    if (offsetTable.size() == 0) RebuildIndex();
    
    if (currentEvt != offsetTable.end()) {
        Event nextEvent = ReadEvent();
        return nextEvent;
    }
    else throw Exceptions::End_of_File();
}

Event EventFile::GetPreviousEvent()
{
    if (!isInitialized) throw Exceptions::Not_Init();
    
    if (currentEvt != offsetTable.begin()) {
        currentEvt--;
        Event prevEvent = ReadEvent();
        return prevEvent;
    }
    else throw Exceptions::End_of_File();
}

bool EventFile::eof()
{
    if (!isInitialized) throw Exceptions::Not_Init();
    return file.eof();
}