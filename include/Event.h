#ifndef EVENT_H
#define EVENT_H

#include <iostream>
#include <vector>
#include "Constants.h"
#include "GRAWFrame.h"
#include "GRAWDataItem.h"
#include "LookupTable.h"
#include "PadLookupTable.h"
#include "GMExceptions.h"
#include <unordered_map>
#include <cmath>
#include "Utilities.h"
#include "UserInterface.h"
#include "HardwareAddress.h"
#include <armadillo>
#include <memory>

/** \brief Representation of an event in the detector.

 This class represents a single event from the DAQ. It is created initially by merging together corresponding frames from the GRAW files.

 */
class Event
{
public:
    using mapType = std::unordered_map<HardwareAddress, arma::Col<sample_t>>;

    // Construction of Events

    //! \brief Default contstructor.
    Event();

    //! \brief Copy constructor
    Event(const Event& orig);

    //! \brief Move constructor
    Event(Event&& orig);

    //! \brief Copy operator
    Event& operator=(const Event& orig);

    //! \brief Move operator
    Event& operator=(Event&& orig);

    mapType::iterator begin();
    mapType::iterator end();
    mapType::const_iterator cbegin() const;
    mapType::const_iterator cend() const;

    // Setting properties

    /** \brief Sets a pointer to the pad lookup table.

     This function must be called in order to use the AppendFrame function. It sets a pointer to a PadLookupTable object that must be initialized separately.

     \param table A pointer to an initialized PadLookupTable.

     */
    void SetLookupTable(const std::shared_ptr<PadLookupTable>& table);

    /** \brief Append a frame to the event.

     The frame should be initialized and filled with data before appending it to the event. This function should be called for each frame that composes the event. The event time and event ID will be checked for each frame. If no frames have been appended to the Event yet, then the event time and event ID will be set by the first frame to be appended. If the time or ID do not match on subsequent frames, an error message will be printed.

     */
    void AppendFrame(const GRAWFrame& frame);

    // Getting properties and members

    /** \brief Get the Trace for the given set of parameters.

     \throws std::out_of_range if the trace is not present in the event.
     */
    arma::Col<sample_t>& GetTrace(addr_t cobo, addr_t asad, addr_t aget, addr_t channel);

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

    size_t numTraces() const;


    // I/O functions

    /** \brief Write the event to an output stream.

     \rst

     The event data is written in the event file format. This is documented at :doc:`event_file.rst`.

     \endrst

     */
    friend std::ostream& operator<<(std::ostream& stream, const Event& event);

    // Event Header fields

    evtid_t eventId;
    ts_t eventTime;

private:

    // Lookup table pointer and hash functions

    std::shared_ptr<PadLookupTable> lookupTable;

    int nFramesAppended;  // The number of frames appended to this event

    // Traces for each pad

    std::unordered_map<HardwareAddress, arma::Col<sample_t>> data;

    friend class EventFile;
    friend class EventTestFixture;
};

#endif /* defined(EVENT_H) */
