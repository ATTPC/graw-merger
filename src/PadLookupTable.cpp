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
