//
//  Event.h
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __GETConsolidate__Event__
#define __GETConsolidate__Event__

#include <iostream>
#include <vector>
#include "GETFrame.h"
#include "GETFrameDataItem.h"
#include "Trace.h"
#include "PadLookupTable.h"
#include "GETExceptions.h"
#include <unordered_map>
#include <cmath>
#include "Utilities.h"

/** \brief Representation of an event in the detector.
 
 This class represents a single event from the DAQ. It is created initially by merging together corresponding frames from the GRAW files.
 
 */
class Event
{
public:
    
    // Construction of Events
    
    //! \brief Default contstructor. 
    Event();
    
    /** \brief Deserialize an event.
     
     Constructs an Event object from its serialized form. This can be used to re-construct an Event object that was previously saved to an Event file.
     
     \param raw The serialized event object, as an array of bytes.
     
     \throws Exceptions::Wrong_File_Position Thrown if the provided bytes don't match the signature of an event.
     
     \rst
     .. NOTE::
        Unlike the :class:`GETFrame` class, the Event class expects its raw data to be *little*-endian. 
     \endrst
     
     */
    Event(const std::vector<uint8_t>& raw);
    
    //! \brief Copy constructor
    Event(const Event& orig);
    
    //! \brief Move constructor
    Event(Event&& orig);
    
    //! \brief Copy operator
    Event& operator=(const Event& orig);
    
    //! \brief Move operator
    Event& operator=(Event&& orig);
    
    // Setting properties
    
    /** \brief Sets a pointer to the pad lookup table.
    
     This function must be called in order to use the AppendFrame function. It sets a pointer to a PadLookupTable object that must be initialized separately.
     
     \param table A pointer to an initialized PadLookupTable.
     
     */
    void SetLookupTable(PadLookupTable* table);
    
    void SetEventId(const uint32_t eventId_in);
    void SetEventTime(const uint64_t eventTime_in);
    
    void AppendFrame(const GETFrame& frame);
    std::vector<GETFrame> ExtractAllFrames();
    
    // Getting properties and members
    
    uint32_t Size() const;
    
    Trace& GetTrace(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel);
    
    uint32_t GetEventId() const;
    uint64_t GetEventTime() const;
    
    // Manipulations of contained data
    
    void SubtractFPN();
    
    // I/O functions
    
    friend std::ostream& operator<<(std::ostream& stream, const Event& event);
    
private:
    
    // Lookup table pointer and hash functions
    
    PadLookupTable *lookupTable;
    int CalculateHash(uint8_t cobo, uint8_t asad, uint8_t aget, uint8_t channel);
    
    int nFramesAppended;  // The number of frames appended to this event
    
    // Event Header fields
    
    uint32_t eventId;
    uint64_t eventTime;
    
    static const uint8_t magic; // Equals 0xEE, defined in Event.cpp
    
    // Traces for each pad
    
    std::unordered_map<int,Trace> traces;
    
    friend class EventFile;
    friend class EventTestFixture;
};

#endif /* defined(__GETConsolidate__Event__) */
