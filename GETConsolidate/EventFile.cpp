//
//  EventFile.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/9/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "EventFile.h"
#include "ExceptionWithString.h"

EventFile::EventFile ()
{
    offsetTable = new std::map<unsigned int,unsigned long long>;
    
    isInitialized = false;
}

EventFile::~EventFile ()
{
    if (file.is_open()) {
        file.close();
    }
    delete offsetTable;
}

void EventFile::CloseFile()
{
    if (file.is_open() and this->isInitialized) file.close();
}

void EventFile::OpenFileForWrite(std::string path)
{
    // Make sure the object isn't already initialized
    
    if (isInitialized) {
        throw ExceptionWithString("Event file object already initialized.");
    }
    
    file.open(path, std::ios::out|std::ios::binary|std::ios::trunc);
    if (!file.good()) {
        throw ExceptionWithString("Failed to open file for write.");
    }
    isInitialized = true;
    
    file.write(magic,sizeof(magic));
}

void EventFile::WriteEvent(Event* event)
{
    // Make sure file is initialized
    
    if (!isInitialized) {
        throw ExceptionWithString("Tried to write event to uninitialized file object.");
    }
    
    unsigned long long currentPos = file.tellg();
    offsetTable->emplace(event->eventId, currentPos);
    
    file << *event;
}