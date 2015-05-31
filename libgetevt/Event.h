//
//  Event.h
//  get-manip
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __get_manip__Event__
#define __get_manip__Event__

#include <iostream>
#include <vector>
#include "Constants.h"
#include "GRAWFrame.h"
#include "GRAWDataItem.h"
#include "Trace.h"
#include "LookupTable.h"
#include "PadLookupTable.h"
#include "GMExceptions.h"
#include <unordered_map>
#include <cmath>
#include "Utilities.h"

namespace getevt {
    /** \brief Representation of an event in the detector.

     This class represents a single event from the DAQ. It is created initially by merging together corresponding frames from the GRAW files.

     */
    class Event
    {
    public:

        // Construction of Events

        //! \brief Default constructor.
        Event();

        /** \brief Deserialize an event.

         Constructs an Event object from its serialized form. This can be used to re-construct an Event object that was previously saved to an Event file.

         \param raw The serialized event object, as an array of bytes.

         \throws Exceptions::Wrong_File_Position Thrown if the provided bytes don't match the signature of an event.

         \rst
         .. NOTE::
            Unlike the :class:`GRAWFrame` class, the Event class expects its raw data to be *little*-endian.
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

        //! \brief Set the event ID for this event.
        void SetEventId(const evtid_t eventId_in);

        //! \brief Set the event time for this event.
        void SetEventTime(const ts_t eventTime_in);

        /** \brief Append a frame to the event.

         The frame should be initialized and filled with data before appending it to the event. This function should be called for each frame that composes the event. The event time and event ID will be checked for each frame. If no frames have been appended to the Event yet, then the event time and event ID will be set by the first frame to be appended. If the time or ID do not match on subsequent frames, an error message will be printed.

         */
        void AppendFrame(const GRAWFrame& frame);

        /** \brief Turns the event back into GRAW frames.

         \rst

         This can be used to produce a file in the original GRAW format.

         ..  WARNING::
             As some of the data from the original file is thrown out, the header information written by this function may not be entirely meaningful. It does try to recreate things like the hit patterns, time stamps, and IDs, but some information like the metaType, status, and other unused fields might be inconsistent with the current GRAW file specifications.

         \endrst

         */
        std::vector<GRAWFrame> ExtractAllFrames();

        // Getting properties and members

        //! \brief Get the size of the event as written to a file, in bytes.
        uint32_t Size() const;

        /** \brief Get the Trace for the given set of parameters.

         \throws std::out_of_range if the trace is not present in the event.
         */
        Trace& GetTrace(addr_t cobo, addr_t asad, addr_t aget, addr_t channel);

        //! \returns The event ID.
        evtid_t GetEventId() const;

        //! \returns The event time.
        ts_t GetEventTime() const;

        // Manipulations of contained data

        /** \brief Subtract the fixed pattern noise (FPN) from the rest of the data

         \rst

         This function goes through the data one AGET at a time. For each AGET, it does the following:

             #. Find the fixed pattern noise channels (channels 11, 22, 45, and 56).

             #. Average them together.

             #. Renormalize this average to zero. (This is necessary due to the arbitrary gain on each channel.)

             #. Subtract this renormalized average from all other channels on this AGET.

         After this process, the FPN channels are deleted from the event. This reduces the size of the output file substantially.

         \endrst

         */
        void SubtractFPN();

        //! \brief Subtract precomputed pedestal values from each trace in the event.
        void SubtractPedestals(const LookupTable<sample_t>& pedsTable);

        /** \brief Applies the provided threshold to the traces in the event.

        Any sample that is less than the threshold is set to zero.

        */
        void ApplyThreshold(const sample_t threshold);

        /** \brief Drops zeros from all traces.

         This function is called to implement zero suppression. It calls the DropZeros function for each trace contained in the event. If a trace is empty after dropping all of its zeros, it will be dropped entirely from the event.

         In general, this should be called right before writing the event to disk.
         */
        void DropZeros();

        // I/O functions

        /** \brief Write the event to an output stream.

         \rst

         The event data is written in the event file format. This is documented at :doc:`event_file.rst`.

         \endrst

         */
        friend std::ostream& operator<<(std::ostream& stream, const Event& event);

    private:

        // Lookup table pointer and hash functions

        PadLookupTable *lookupTable;
        hash_t CalculateHash(addr_t cobo, addr_t asad, addr_t aget, addr_t channel);

        int nFramesAppended;  // The number of frames appended to this event

        // Event Header fields

        evtid_t eventId;
        ts_t eventTime;

        static const uint8_t magic; // Equals 0xEE, defined in Event.cpp

        // Traces for each pad

        std::unordered_map<hash_t,Trace> traces;

        friend class EventFile;
        friend class EventTestFixture;
    };
}

#endif /* defined(__get_manip__Event__) */