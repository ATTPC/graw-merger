//
//  PadLookupTable.h
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/6/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __GETConsolidate__PadLookupTable__
#define __GETConsolidate__PadLookupTable__

#include <iostream>
#include <fstream>
#include <map>

class PadLookupTable
{
    /* This class represents a lookup table that maps cobo, asad, aget, and 
     * channel onto a pad number. This is dependent on the wiring of the ATTPC,
     * and such a table can be generated using, for example, Igor Pro. For the
     * purposes of this code, we assume that the table has already been made and
     * is in an external file. The file name has to be passed to the constructor
     * of this function, and it will be opened and read in.
     */
    
public:
    PadLookupTable(const std::string& path);
    /* The constructor takes a string argument that specifies where on disk the 
     * lookup table file is located. The file should be in csv format, with data
     * in the order
     *      cobo,asad,aget,channel,pad
     * There should be no headers in the file.
     *
     * One big caveat: Igor's output files have classic Mac OS \r line endings. 
     * This program assumes standard Unix \n line endings. This could easily be
     * changed, but at the expense of using the standard. I've chosen to change 
     * the line endings in the Igor file manually instead.
     */
    
    uint16_t FindPadNumber(uint8_t cobo, uint8_t asad, uint8_t aget,
                           uint8_t channel);
    /* This function looks up a pad number in the table using the information 
     * passed to the function.
     */
    
private:
    uint32_t CalculateHash(uint8_t cobo, uint8_t asad, uint8_t aget,
                           uint8_t channel);
    /* This is the hashing function for the hashtable used to store the pad 
     * numbers. It simply concatenates cobo+asad+aget+channel since this is 
     * unique for each pad.
     */
    
    std::map<uint32_t,uint16_t> table;  // The hashtable, maps hash:pad#
};

#endif /* defined(__GETConsolidate__PadLookupTable__) */
