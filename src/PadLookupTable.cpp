//
//  PadLookupTable.cpp
//  get-manip
//
//  Created by Joshua Bradt on 12/27/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "PadLookupTable.h"

PadLookupTable::PadLookupTable()
: LookupTable ()
{
    missingValue = 20000;
}

PadLookupTable::PadLookupTable(const std::string& file)
: LookupTable (file)
{
    missingValue = 20000;
}
