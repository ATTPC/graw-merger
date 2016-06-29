#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

typedef uint64_t ts_t;
typedef uint32_t evtid_t;
typedef uint8_t addr_t;
typedef uint16_t pad_t;
typedef uint16_t tb_t;
typedef int16_t sample_t;

typedef uint32_t hash_t;

namespace Constants {
    // Sizes
    static const addr_t num_cobos {10};
    static const addr_t num_asads {4};
    static const addr_t num_agets {4};
    static const addr_t num_channels {68};
    static const tb_t   num_tbs {512};
    static const pad_t  num_pads {10240};

    static const sample_t min_sample {INT16_MIN};
    static const sample_t max_sample {INT16_MAX};
}

#endif
