#ifndef PADLOOKUPTABLE_H
#define PADLOOKUPTABLE_H

#include "LookupTable.h"
#include "Constants.h"

/** \brief Used to look up pad numbers.

 This class represents a lookup table that maps cobo, asad, aget, and channel onto a pad number. This is dependent on the wiring of the ATTPC, and such a table can be generated using, for example, Igor Pro. For the purposes of this code, we assume that the table has already been made and is in an external file. The file name has to be passed to the constructor of this function, and it will be opened and read in.
 */
using PadLookupTable = LookupTable<pad_t, 20000>;

#endif /* defined(PADLOOKUPTABLE_H) */
