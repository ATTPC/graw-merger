//
//  EventFile.h
//  get-manip
//
//  Created by Joshua Bradt on 5/9/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __get_manip__EventFile__
#define __get_manip__EventFile__

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>

#include "Event.h"
#include "GMExceptions.h"
#include "DataFile.h"
#include "Constants.h"

/** \brief Interface to an Event file.
 
 \rst
 
 This subclass of class DataFile is used to interface with an Event file on disk. Primarily, this is useful as output when merging many GRAW files together.
  
 \endrst
 
 */
class EventFile : public DataFile
{
public:
    // Opening and closing the file. These override DataFile
    
    /** \brief Open the file for output.
     
     In addition to opening the file, this function also writes the event file magic number to the beginning of the file for later identification.
     
     */
    void OpenFileForWrite(const std::string& path) override;
    
    //! \overload
    void OpenFileForWrite(const boost::filesystem::path& path) override;
    
    /** \brief Open the file for input.
     
     After opening the file, this function checks for the event file magic number. If this value is not found at the beginning of the file, an exception is thrown.
     
     \throws Exceptions::Wrong_File_Type Thrown if the magic number is not found at the beginning of the file.
     
     */
    void OpenFileForRead(const std::string& path) override;
    
    //! \overload
    void OpenFileForRead(const boost::filesystem::path& path) override;
    
    // Functions for writing events to file

    /** \brief Writes the given event to the file.
     
     Internally, this function calls the ostream insertion operator (operator<<) for the event. For details about this operator, see the corresponding entry for the Event class.
     
     \throws Exceptions::Not_Init Thrown if the file is not initialized by one of the OpenFile methods before use.
     
     */
    void WriteEvent(const Event& event);
    
    // Functions for reading events from a file

    /** \brief Rebuild the index of event positions within the file.
     
     To allow random access to events in an event file, this class maintains an index of the file position of each event in a file. This index can be constructed upon opening a file by calling this method.
     
     \throws Exceptions::Not_Init Thrown if the file is not initialized by an OpenFile method.
     
     \throws Exceptions::Wrong_File_Position Thrown if the event magic number is not found at what should be the beginning of an event. This could indicate that the event lengths written in the file are wrong.
     
     */
    void RebuildIndex();
    
    /** \brief Extract the next serialized event from the file.
     
     This reads in the next event from the file, and stores the result in a vector of bytes that can be passed to the constructor of the Event class to de-serialize the event.
     
     This function will also automatically call RebuildIndex() if the event index does not exist.
     
     \return A vector containing the raw data.
     
     \throws Exceptions::Not_Init If the file is not initialized by one of the OpenFile methods.
     
     \throws Exceptions::Wrong_File_Position If the beginning of an event cannot be found.
     
     */
    std::vector<uint8_t> ReadRawFrame() override;
    
    //! \brief Get the next event listed in the index.
    Event GetNextEvent();
    
    //! \brief Get the previous event listed in the index.
    Event GetPreviousEvent();
    
    //    Event GetEventByNumber(const unsigned int evtNumber);
    
private:
    std::map<evtid_t,std::streamoff> offsetTable; // evt# : offset
    std::map<evtid_t,std::streamoff>::iterator currentEvt;
    
    // Magic string for file ID
    
    static const int magic;
};

#endif /* defined(__get_manip__EventFile__) */
