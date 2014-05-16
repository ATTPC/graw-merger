//
//  EventFile.h
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/9/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __GETConsolidate__EventFile__
#define __GETConsolidate__EventFile__

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "Event.h"
#include "GETExceptions.h"

class EventFile
{
public:
    
    // Constructors, destructor, copy, and move
    // (copy and move are deleted since they wouldn't make sense)
    
    EventFile();
    ~EventFile();
    
    EventFile(const EventFile& orig) =delete;
    EventFile(EventFile&& orig) =delete;
    
    EventFile& operator=(const EventFile& other) =delete;
    EventFile& operator=(EventFile&& other) =delete;
    
    // Opening and closing the file
    
    void OpenFileForWrite(std::string path);
    void OpenFileForRead(const std::string path);
    void CloseFile();
    
    // Functions for writing events to file

    void WriteEvent(const Event& event);
    
    // Functions for reading events from a file

    void RebuildIndex();
    Event ReadEvent();
    Event GetNextEvent();
    Event GetPreviousEvent();
    //    Event GetEventByNumber(const unsigned int evtNumber);
    bool eof();
    
private:
    std::map<unsigned int,unsigned long long> offsetTable; // evt# : offset
    std::map<unsigned int,unsigned long long>::iterator currentEvt;
    
    std::fstream file;
    
    // Magic string for file ID
    
    static const int magic;
    
    bool isInitialized = false;
    
};

#endif /* defined(__GETConsolidate__EventFile__) */
