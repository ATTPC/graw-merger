#include "GRAWFrame.h"

// --------
// Static constants
// --------

const uint8_t  GRAWFrame::Expected_metaType = 8;
const uint16_t GRAWFrame::Expected_headerSize = 1;
const uint16_t GRAWFrame::Expected_itemSizePartialReadout = 4;
const uint16_t GRAWFrame::Expected_itemSizeFullReadout = 2;
const uint16_t GRAWFrame::Expected_frameTypePartialReadout = 1;
const uint16_t GRAWFrame::Expected_frameTypeFullReadout = 2;
const int      GRAWFrame::sizeUnit = 256;

// --------
// Constructor
// --------

GRAWFrame::GRAWFrame()
: metaType(6),frameSize(0),dataSource(0),frameType(1),revision(4),headerSize(2),itemSize(4),nItems(0),eventTime(0),eventId(0),coboId(0),asadId(0),readOffset(0),status(0)
{
    hitPatterns = {0,0,0,0};
    multiplicity = {0,0,0,0};
}

GRAWFrame::GRAWFrame(const RawFrame& rawFrame)
{
    auto rawFrameIter = rawFrame.begin();

    metaType = *rawFrameIter;
    rawFrameIter++;
    if (metaType != Expected_metaType) {
        BOOST_LOG_TRIVIAL(warning) << "Unexpected metaType " << int(metaType);
    }

    frameSize = Utilities::ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter + 3);
    rawFrameIter += 3;
    if (frameSize*sizeUnit != rawFrame.size()) {
        BOOST_LOG_TRIVIAL(warning) << "Wrong frameSize. Using raw frame size.";
        frameSize = static_cast<decltype(frameSize)>(rawFrame.size()/sizeUnit);
    }

    dataSource = *rawFrameIter;
    rawFrameIter++;

    frameType = Utilities::ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter += 2;
    if (frameType != Expected_frameTypeFullReadout and
        frameType != Expected_frameTypePartialReadout) {
        BOOST_LOG_TRIVIAL(warning) << "Unknown frameType. Read will likely fail.";
    }

    revision = *rawFrameIter;
    rawFrameIter++;

    headerSize = Utilities::ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter += 2;
    if (headerSize != Expected_headerSize) {
        BOOST_LOG_TRIVIAL(warning) << "Wrong headerSize " << int(headerSize) << ". Correcting.";
        headerSize = Expected_headerSize;
    }

    itemSize = Utilities::ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter += 2;
    if ((frameType == Expected_frameTypePartialReadout and
         itemSize != Expected_itemSizePartialReadout) or
        (frameType == Expected_frameTypeFullReadout and
         itemSize != Expected_itemSizeFullReadout)) {
            BOOST_LOG_TRIVIAL(warning) << "Wrong itemSize " << int(itemSize) << ". Correcting.";
            if (frameType == Expected_frameTypePartialReadout) {
                itemSize = Expected_itemSizePartialReadout;
            }
            else if (frameType == Expected_frameTypeFullReadout) {
                itemSize = Expected_itemSizeFullReadout;
            }
    }

    nItems = Utilities::ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter+4);
    rawFrameIter += 4;
    if (frameSize != ceil(double(nItems*itemSize + headerSize*sizeUnit)/sizeUnit)) {
        BOOST_LOG_TRIVIAL(warning) << "Mismatched nItems. Correcting.";
        nItems = (frameSize*sizeUnit - headerSize*sizeUnit)/itemSize;
    }

    eventTime = Utilities::ExtractByteSwappedInt<uint64_t>(rawFrameIter, rawFrameIter+6);
    rawFrameIter += 6;

    eventId = Utilities::ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter+4);
    rawFrameIter += 4;

    coboId = *rawFrameIter;
    rawFrameIter++;

    asadId = *rawFrameIter;
    rawFrameIter++;

    readOffset = Utilities::ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+2);
    rawFrameIter+=2;

    status = *rawFrameIter;
    rawFrameIter++;

    for (int aget = 0; aget<4; aget++) {
        std::bitset<9*8> bs {};   // init to 0
        for (int byte = 8; byte >= 0; byte--) {
            std::bitset<72> temp {*rawFrameIter};
            temp <<= static_cast<size_t>(byte*8);
            bs |= temp;
            rawFrameIter++;
        }
        hitPatterns.push_back(bs);
    }

    for (int aget = 0; aget<4; aget++) {
        uint16_t mult_in = Utilities::ExtractByteSwappedInt<decltype(mult_in)>(rawFrameIter, rawFrameIter + sizeof(mult_in));
        rawFrameIter += sizeof(mult_in);
        multiplicity.push_back(mult_in);
    }

    // Extract data items
    auto dataBegin = rawFrame.begin() + headerSize*GRAWFrame::sizeUnit;
    auto dataEnd   = dataBegin + nItems*itemSize;

    if (frameType == Expected_frameTypePartialReadout) {
        ExtractPartialReadoutData(dataBegin, dataEnd);
    }
    else if (frameType == Expected_frameTypeFullReadout) {
        ExtractFullReadoutData(dataBegin, dataEnd);
    }
}

// --------
// Iteration
// --------

std::vector<GRAWDataItem>::iterator GRAWFrame::begin()
{
    return data.begin();
}

std::vector<GRAWDataItem>::iterator GRAWFrame::end()
{
    return data.end();
}

std::vector<GRAWDataItem>::const_iterator GRAWFrame::cbegin() const
{
    return data.cbegin();
}

std::vector<GRAWDataItem>::const_iterator GRAWFrame::cend() const
{
    return data.cend();
}

// --------
// Data Extraction Functions
// --------

template <typename charIter>
void GRAWFrame::ExtractPartialReadoutData(charIter& begin, charIter& end)
{
    std::vector< std::bitset<72> > actualHitPattern {0,0,0,0};

    for (auto rawFrameIter = begin; rawFrameIter != end; rawFrameIter+=itemSize) {
        uint32_t item = Utilities::ExtractByteSwappedInt<uint32_t>(rawFrameIter, rawFrameIter+itemSize);

        addr_t aget    = ExtractAgetId(item);
        if (aget >= Constants::num_agets) {
            BOOST_LOG_TRIVIAL(warning) << "Frame contains invalid AGET value " << std::to_string(aget);
            continue;
        }

        addr_t channel = ExtractChannel(item);
        if (channel >= Constants::num_channels) {
            BOOST_LOG_TRIVIAL(warning) << "Frame contains invalid channel " << std::to_string(channel);
            continue;
        }

        tb_t tbid   = ExtractTBid(item);
        if (tbid >= Constants::num_tbs) {
            BOOST_LOG_TRIVIAL(warning) << "Frame contains invalid TB " << std::to_string(tbid);
            continue;
        }

        sample_t sample  = ExtractSample(item);

        data.push_back(GRAWDataItem(aget,channel,tbid,sample));

        // WARNING: The hit pattern is in the reverse order of the bitset's accessor.

        actualHitPattern.at(aget).set(67-channel);
    }

    // Compare hit patterns

    unsigned int nMissing = 0;
    unsigned int nUnexpected = 0;

    for (size_t aget_iter = 0; aget_iter < 4; aget_iter++) {
        for (size_t ch_iter = 0; ch_iter < 68; ch_iter ++) {
            bool isExpected = hitPatterns.at(aget_iter).test(ch_iter);
            bool isFound = actualHitPattern.at(aget_iter).test(ch_iter);

            if (isExpected and !isFound) {
                nMissing++;
            }
            else if (isFound and !isExpected) {
                nUnexpected++;
            }
        }
    }

    if (data.size() != nItems) {
        BOOST_LOG_TRIVIAL(warning) << "Missing data items";
    }

}

template <typename charIter>
void GRAWFrame::ExtractFullReadoutData(charIter& begin, charIter& end)
{
//    std::vector<std::queue<int16_t>> dataQueues (4);
    std::vector<std::array<int16_t,34816>> dataArrays (4);
    uint16_t i[4]={0,0,0,0};

    for (auto rawFrameIter = begin; rawFrameIter != end; rawFrameIter += itemSize) {
        uint16_t item = Utilities::ExtractByteSwappedInt<uint16_t>(rawFrameIter, rawFrameIter+itemSize);

        addr_t aget    = ExtractAgetIdFullReadout(item);
        if (aget >= Constants::num_agets) {
            BOOST_LOG_TRIVIAL(warning) << "Invalid AGET value " << std::to_string(aget);
        }
        sample_t sample  = ExtractSampleFullReadout(item);

//        dataQueues.at(aget).push(sample);
        dataArrays.at(aget).at(i[aget]) = sample;
        i[aget]++;
        
    }

	i[0]=0;i[1]=0;i[2]=0;i[3]=0;
    for (addr_t aget = 0; aget < 4; aget++) {
        for (tb_t tbid = 0; tbid < 512; tbid ++) {
            for (addr_t channel = 0; channel < 68; channel++) {
//                auto sample = dataQueues.at(aget).front();
//                dataQueues.at(aget).pop();
				auto sample = dataArrays.at(aget).at(i[aget]);
				i[aget]++;


                data.push_back(GRAWDataItem(aget,channel,tbid,sample));
            }
        }
    }
}

addr_t GRAWFrame::ExtractAgetId(const uint32_t raw)
{
    return (raw & 0xC0000000)>>30;
}

addr_t GRAWFrame::ExtractChannel(const uint32_t raw)
{
    return (raw & 0x3F800000)>>23;
}

tb_t GRAWFrame::ExtractTBid(const uint32_t raw)
{
    return (raw & 0x007FC000)>>14;
}

sample_t GRAWFrame::ExtractSample(const uint32_t raw)
{
    return (raw & 0x00000FFF);
}

addr_t GRAWFrame::ExtractAgetIdFullReadout(const uint16_t raw)
{
    return (raw & 0xC000)>>14;
}

sample_t GRAWFrame::ExtractSampleFullReadout(const uint16_t raw)
{
    return (raw & 0x0FFF);
}
