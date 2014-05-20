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
#include <boost/filesystem.hpp>

#include "Event.h"
#include "GETExceptions.h"
#include "DataFile.h"

class EventFile : public DataFile
{
public:
    // Opening and closing the file. These override DataFile
    
    void OpenFileForWrite(const std::string& path) override;
    void OpenFileForWrite(const boost::filesystem::path& path) override;
    void OpenFileForRead(const std::string& path) override;
    void OpenFileForRead(const boost::filesystem::path& path) override;
    
    // Functions for writing events to file

    void WriteEvent(const Event& event);
    
    // Functions for reading events from a file

    void RebuildIndex();
    std::vector<uint8_t> ReadRawFrame() override;
    
    Event GetNextEvent();
    Event GetPreviousEvent();
    //    Event GetEventByNumber(const unsigned int evtNumber);
    
private:
    std::map<unsigned int,unsigned long long> offsetTable; // evt# : offset
    std::map<unsigned int,unsigned long long>::iterator currentEvt;
    
    // Magic string for file ID
    
    static const int magic;
};

#endif /* defined(__GETConsolidate__EventFile__) */
