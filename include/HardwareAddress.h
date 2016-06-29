#ifndef HARDWAREADDRESS_H
#define HARDWAREADDRESS_H

#include "Constants.h"
#include <unordered_map>

struct HardwareAddress
{
    addr_t cobo;
    addr_t asad;
    addr_t aget;
    addr_t channel;
    pad_t pad;

    bool operator==(const HardwareAddress& other) const
    {
        return (cobo == other.cobo) && (asad == other.asad) && (aget == other.aget)
                && (channel == other.channel) && (pad == other.pad);
    }
};

template<>
struct std::hash<HardwareAddress>
{
    size_t operator()(const HardwareAddress& addr) const
    {
        // Widen the integers so they don't overflow on multiplication
        size_t wcobo = addr.cobo;
        size_t wasad = addr.asad;
        size_t waget = addr.aget;
        size_t wchannel = addr.channel;

        size_t result = wchannel + waget*100 + wasad*10000 + wcobo*1000000;

        return result;
    }
};

#endif /* end of include guard: HARDWAREADDRESS_H */
