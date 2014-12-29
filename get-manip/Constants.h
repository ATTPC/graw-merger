//
//  Constants.h
//  get-manip
//
//  Created by Joshua Bradt on 12/28/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef get_manip_Constants_h
#define get_manip_Constants_h

typedef uint64_t ts_t;
typedef uint32_t evtid_t;
typedef uint8_t addr_t;
typedef uint16_t pad_t;
typedef uint16_t tb_t;
typedef int16_t sample_t;

namespace Constants {
    // Sizes
    static const addr_t num_cobos {10};
    static const addr_t num_asads {4};
    static const addr_t num_agets {4};
    static const addr_t num_channels {68};
    static const tb_t   num_tbs {512};
}

#endif