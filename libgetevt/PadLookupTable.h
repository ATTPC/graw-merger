//
//  PadLookupTable.h
//  get-manip
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __get_manip__PadLookupTable__
#define __get_manip__PadLookupTable__

#include "LookupTable.h"
#include "Constants.h"


namespace getevt
{
    /** \brief Used to look up pad numbers.

     This class represents a lookup table that maps cobo, asad, aget, and channel onto a pad number.
     This is dependent on the wiring of the ATTPC, and such a table can be generated using, for example, Igor Pro.
     For the purposes of this code, we assume that the table has already been made and is in an external file.
     The file name has to be passed to the constructor of this function, and it will be opened and read in.
     */
    class PadLookupTable : public LookupTable<pad_t>
    {
    public:

        PadLookupTable();

        PadLookupTable(const std::string& file);

        struct Address
        {
            addr_t cobo{0};
            addr_t asad{0};
            addr_t aget{0};
            addr_t channel{0};

            Address()
            { };

            Address(addr_t cobo, addr_t asad, addr_t aget, addr_t channel)
                    : cobo(cobo), asad(asad), aget(aget), channel(channel)
            { };
        };

        Address ReverseFind(const pad_t pad) const;

    private:

        friend class PadLookupTableTestFixture;

        std::unordered_map<pad_t, Address> reverse_table;

    };
}

#endif /* defined(__get_manip__PadLookupTable__) */
