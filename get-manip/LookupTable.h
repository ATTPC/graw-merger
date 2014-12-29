//
//  LookupTable.h
//  get-manip
//
//  Created by Joshua Bradt on 12/27/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __get_manip__LookupTable__
#define __get_manip__LookupTable__

#include <iostream>
#include <fstream>
#include <unordered_map>
#include "Constants.h"

/** \brief A base class representing a lookup table.
 
 This class represents a lookup table that maps cobo, asad, aget, and channel onto some value. This can be used to look up, for example, pad numbers or pedestals.
 
 For the purposes of this code, we assume that the table has already been made and is in an external file. The file name has to be passed to the constructor of this class, and it will be opened and read in.
 
 The attribute ``missingValue`` should be set to an appropriate value in a derived class.
 */
template <typename mapped_t>
class LookupTable
{
public:
    /** \brief Load from CSV file
     The constructor takes a string argument that specifies where on disk the lookup table file is located. The file should be in csv format, with data in the order cobo, asad, aget, channel, value. There should be no headers in the file.
     
     \rst
     .. WARNING::
        Igor's output files have classic Mac OS `\\r` line endings. This program assumes standard Unix `\\n` line endings. This could easily be changed, but at the expense of using the standard. I've chosen to change the line endings in the Igor file manually instead.
     \endrst
     */
    LookupTable(const std::string& path);
    
    //! \brief A default constructor (for testing only)
    LookupTable();

    //! \brief Looks up a pad number in the table using the information passed to the function.
    mapped_t Find(addr_t cobo, addr_t asad, addr_t aget, addr_t channel) const;
    
    //! \brief Tests if the table is empty.
    bool Empty() const;
   
    //! \brief Read the file at the provided path and replace all values in the hash table with values from the file.
    void ReadFile(const std::string& path);
    
    //! \brief The value returned when a pad is missing from the lookup table.
    mapped_t missingValue {0};
    
protected:
    /** \brief Hashing function for the hashtable used to store the values.
     
     It simply concatenates cobo+asad+aget+channel since this is unique for each value.
     */
    static hash_t CalculateHash(addr_t cobo, addr_t asad, addr_t aget, addr_t channel);
    
    std::unordered_map<hash_t,mapped_t> table;  // The hashtable, maps hash:value
    
    friend class EventTestFixture;
};

#endif /* defined(__get_manip__LookupTable__) */
