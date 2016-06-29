#include "Event.h"

// --------
// Constructors, Move, and Copy
// --------

Event::Event()
: eventId(0),eventTime(0),lookupTable(nullptr),nFramesAppended(0)
{
}

Event::Event(const Event& orig)
: eventId(orig.eventId),eventTime(orig.eventTime),lookupTable(orig.lookupTable),nFramesAppended(orig.nFramesAppended)
{
    data = orig.data;
}

Event::Event(Event&& orig)
: eventId(orig.eventId),eventTime(orig.eventTime),lookupTable(orig.lookupTable),nFramesAppended(orig.nFramesAppended)
{
    data = std::move(orig.data);
}

Event& Event::operator=(const Event& orig)
{
    this->lookupTable = orig.lookupTable;  // could leak
    this->eventId = orig.eventId;
    this->eventTime = orig.eventTime;
    this->nFramesAppended = orig.nFramesAppended;

    this->data.clear();
    this->data = orig.data;

    return *this;
}

Event& Event::operator=(Event&& orig)
{
    this->lookupTable = orig.lookupTable;  // could leak
    this->eventId = orig.eventId;
    this->eventTime = orig.eventTime;
    this->nFramesAppended = orig.nFramesAppended;

    this->data.clear();
    this->data = std::move(orig.data);

    return *this;
}

Event::mapType::iterator Event::begin()
{
    return data.begin();
}

Event::mapType::iterator Event::end()
{
    return data.end();
}

Event::mapType::const_iterator Event::cbegin() const
{
    return data.cbegin();
}

Event::mapType::const_iterator Event::cend() const
{
    return data.cend();
}

// --------
// Setting Properties
// --------

void Event::SetLookupTable(PadLookupTable * table)
{
    lookupTable = table;
}

void Event::AppendFrame(const GRAWFrame& frame)
{
    // Make sure pointers to required objects are valid

    if (lookupTable == NULL) {
        LOG_ERROR << "No lookup table provided to Event." << std::endl;
        throw Exceptions::Not_Init();
    }

    // Get header information from frame

    addr_t cobo = frame.coboId;
    addr_t asad = frame.asadId;

    if (nFramesAppended == 0) {
        this->eventId = frame.eventId;
    }
    else if (this->eventId != frame.eventId) {
        LOG_WARNING << "Event ID mismatch: CoBo " << cobo << ", AsAd " << asad << std::endl;
    }

    if (nFramesAppended == 0) {
        this->eventTime = frame.eventTime;
    }

    nFramesAppended++;

    // Extract data items and create traces for them

    for (auto frameIter = frame.cbegin(); frameIter < frame.cend(); frameIter++) {
        const GRAWDataItem& dataItem = *frameIter;
        // Extract information
        auto aget = dataItem.agetId;
        auto channel = dataItem.channel;
        auto tbid = dataItem.timeBucketId;
        auto sample = dataItem.sample;

        auto pad = lookupTable->Find(cobo, asad, aget, channel);

        HardwareAddress hwaddr = {cobo, asad, aget, channel, pad};

        // Find trace in hash table, if it exists
        arma::Col<sample_t>& tr = data[hwaddr];

        if (tr.is_empty()) {
            // The trace was just default-constructed, so it's new
            tr.set_size(Constants::num_tbs);
        }

        tr(tbid) = sample;
    }
}

// --------
// Getting Properties and Members
// --------

arma::Col<sample_t>& Event::GetTrace(addr_t cobo, addr_t asad, addr_t aget, addr_t channel)
{
    pad_t pad = lookupTable->Find(cobo, asad, aget, channel);
    HardwareAddress hwaddr {cobo, asad, aget, channel, pad};
    return data.at(hwaddr);  // could throw out_of_range
}

size_t Event::numTraces() const
{
    return data.size();
}

// --------
// Manipulation of Contained Data
// --------

static void renormalizeVectorToZero(arma::Col<sample_t> v)
{
    arma::Col<sample_t> nz = arma::nonzeros(v);
    int64_t total = arma::accu(nz);
    int64_t nzCount = static_cast<int64_t>(nz.n_elem);
    sample_t mean = narrow_cast<sample_t>(total / nzCount);

    v -= mean;
}

void Event::SubtractFPN()
{
    std::vector<addr_t> fpn_channels {11,22,45,56};  // from AGET Docs

    for (addr_t cobo = 0; cobo < Constants::num_cobos; cobo++) {
        for (addr_t asad = 0; asad < Constants::num_asads; asad++) {
            for (addr_t aget = 0; aget < Constants::num_agets; aget++) {

                // Get the FPN channels and find the mean.
                // Each FPN channel may be missing different time buckets, so
                // count the denom of the mean separately for each TB.

                arma::Col<sample_t> mean_fpn (Constants::num_tbs, arma::fill::zeros);
                arma::Col<int> tb_multip (Constants::num_tbs, arma::fill::zeros);

                int num_fpns = 0;

                for (auto ch : fpn_channels) {
                    try {
                        arma::Col<sample_t>& tr = GetTrace(cobo, asad, aget, ch);
                        mean_fpn += tr;
                        for (arma::uword i = 0; i < mean_fpn.n_elem; i++) {
                            if (tr(i) != 0) {
                                tb_multip(i) += 1;
                            }
                        }
                        num_fpns++;
                    }
                    catch (std::out_of_range&) {
                        continue;
                    }
                }

                // Check if there's any FPN data. If not, skip the next part.

                if (num_fpns == 0) continue;

                // Divide by the multiplicity if that value is nonzero.
                for (arma::uword i = 0; i < mean_fpn.n_elem; i++) {
                    if (tb_multip(i) != 0) {
                        mean_fpn(i) /= tb_multip(i);
                    }
                }

                // Renormalize mean FPN to zero
                renormalizeVectorToZero(mean_fpn);

                // Now subtract this mean from the other channels, binwise.
                // This iteration includes the FPN channels.

                for (addr_t ch = 0; ch < Constants::num_channels; ch++) {
                    try {
                        auto tr = GetTrace(cobo, asad, aget, ch);
                        tr -= mean_fpn;
                    }
                    catch (const std::out_of_range&) {
                        continue;
                    }
                }

                // Finally, kill the traces that represent the FPN, since
                // we don't need them for anything else

                for (auto ch : fpn_channels) {
                    data.erase(HardwareAddress{cobo, asad, aget, ch, lookupTable->missingValue});
                }
            }
        }
    }
}
