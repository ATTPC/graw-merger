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

#include "Event.h"

class EventFile
{
private:
    std::map<unsigned int,unsigned long long> *offsetTable; // evt# : offset
    std::fstream file;
    
    // Magic string for file ID: EVTFILE\0
    
    const char* magic = "EVTFILE"; // 8 char, null-term
    
    bool isInitialized;
    
public:
    EventFile();
    ~EventFile();
    
    void CloseFile();
    
    // Functions for writing events to file
    void OpenFileForWrite(std::string path);
    void WriteEvent(Event* event);
    
    // Functions for reading events from a file
    void OpenFileForRead(std::string path);
    Event* GetNextEvent();
    Event* GetPreviousEvent();
    Event* GetEventByNumber(unsigned int evtNumber);
    

};

#endif /* defined(__GETConsolidate__EventFile__) */
