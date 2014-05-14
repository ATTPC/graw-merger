//
//  EventFile.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/9/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "EventFile.h"
#include "GETExceptions.h"

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
    
    file.write(magic,sizeof(magic));
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