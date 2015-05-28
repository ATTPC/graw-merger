//
//  PadLookupTable.cpp
//  get-manip
//
//  Created by Joshua Bradt on 12/27/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "PadLookupTable.h"
#include "GMExceptions.h"

using namespace getevt;

PadLookupTable::PadLookupTable()
: LookupTable ()
{
    missingValue = 20000;
}

PadLookupTable::PadLookupTable(const std::string& file)
: LookupTable (file)
{
    missingValue = 20000;

    // Build the inverse lookup table

    for (addr_t cobo = 0; cobo < Constants::num_cobos; cobo++) {
        for (addr_t asad = 0; asad < Constants::num_asads; asad++) {
            for (addr_t aget = 0; aget < Constants::num_agets; aget++) {
                for (addr_t channel = 0; channel < Constants::num_channels; channel++) {
                    auto pad = this->Find(cobo, asad, aget, channel);
                    if (pad != missingValue) {
                        Address a {cobo, asad, aget, channel};
                        reverse_table.emplace(pad, a);
                    }
                }
            }
        }
    }
}

PadLookupTable::Address PadLookupTable::ReverseFind(const pad_t pad) const
{
    if (!reverse_table.empty()) {
        auto foundItem = reverse_table.find(pad);
        if (foundItem != reverse_table.end()) {
            return foundItem->second;
        }
        else {
            throw Exceptions::Pad_Not_Mapped(pad);
        }
    }
    else {
        throw Exceptions::Not_Init();
    }
}
