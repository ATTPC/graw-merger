#ifndef GRAWFRAME_H
#define GRAWFRAME_H

#include <iostream>
#include <vector>
#include <queue>
#include <bitset>
#include <cmath>
#include <assert.h>
#include <boost/log/trivial.hpp>

#include "GRAWDataItem.h"
#include "GMExceptions.h"
#include "Utilities.h"
#include "Constants.h"
#include "RawFrame.h"

class GRAWFrame
{
public:
    GRAWFrame();
    GRAWFrame(const RawFrame& rawFrame);

    // Iteration

    std::vector<GRAWDataItem>::iterator begin();
    std::vector<GRAWDataItem>::iterator end();
    std::vector<GRAWDataItem>::const_iterator cbegin() const;
    std::vector<GRAWDataItem>::const_iterator cend() const;

    // Data extraction functions

    template <typename charIter>
    void ExtractPartialReadoutData(charIter& begin, charIter& end);

    template <typename charIter>
    void ExtractFullReadoutData(charIter& begin, charIter& end);

    static addr_t ExtractAgetId(const uint32_t raw);
    static addr_t ExtractChannel(const uint32_t raw);
    static tb_t ExtractTBid(const uint32_t raw);
    static sample_t ExtractSample(const uint32_t raw);
    static addr_t ExtractAgetIdFullReadout(const uint16_t raw);
    static sample_t ExtractSampleFullReadout(const uint16_t raw);

    static const uint8_t  Expected_metaType;
    static const uint16_t Expected_headerSize;
    static const uint16_t Expected_itemSizePartialReadout;
    static const uint16_t Expected_itemSizeFullReadout;
    static const uint16_t Expected_frameTypePartialReadout;
    static const uint16_t Expected_frameTypeFullReadout;
    static const int      sizeUnit;

    // Header fields

    uint8_t metaType; // set to 0x6
    uint32_t frameSize; // in units of 64 bytes
    uint8_t dataSource;
    uint16_t frameType;
    uint8_t revision;
    uint16_t headerSize;
    uint16_t itemSize;
    uint32_t nItems;
    uint64_t eventTime;
    uint32_t eventId;
    uint8_t coboId;
    uint8_t asadId;
    uint16_t readOffset;
    uint8_t status;
//    uint8_t hitPattern[4][9];
    std::vector< std::bitset<9*8> > hitPatterns;
//    uint16_t multiplicity[4];
    std::vector<uint16_t> multiplicity;

private:
    // Data items

    std::vector<GRAWDataItem> data;

    // Friends

    friend class Event;
    friend class GRAWFrameTestFixture;
    friend class GRAWFile;
};

#endif
