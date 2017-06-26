#ifndef LOOKUPTABLE_H
#define LOOKUPTABLE_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include "Constants.h"

/** \brief A base class representing a lookup table.

  This class represents a lookup table that maps cobo, asad, aget, and channel onto some value. This can be used to look up, for example, pad numbers or pedestals.

  For the purposes of this code, we assume that the table has already been made and is in an external file. The file name has to be passed to the constructor of this class, and it will be opened and read in.
  */
template <typename mapped_t, mapped_t missingValue_=0>
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
    LookupTable(const std::string& path)
    {
        ReadFile(path);
    }

    //! \brief A default constructor (for testing only)
    LookupTable() {}

    //! \brief Looks up a pad number in the table using the information passed to the function.
    mapped_t Find(addr_t cobo, addr_t asad, addr_t aget, addr_t channel) const
    {
        if (!table.empty()) {
            auto hash = CalculateHash(cobo, asad, aget, channel);
            auto foundItem = table.find(hash);
            if (foundItem != table.end()) {
                return foundItem->second;
            }
            else {
                return missingValue; // an invalid value
            }
        }
        else {
            throw Exceptions::Not_Init();
        }
    }

    //! \brief Tests if the table is empty.
    bool Empty() const
    {
        return table.empty();
    }

    //! \brief Read the file at the provided path and replace all values in the hash table with values from the file.
    void ReadFile(const std::string& path)
    {
        std::ifstream file (path, std::ios::in|std::ios::binary);

        // MUST throw out the first two junk lines in file. No headers!

        if (!file.good()) throw 0; // FIX THIS!

        if (table.size() != 0) {
            table.clear();
        }

        std::string line;

        while (!file.eof()) {
            addr_t cobo, asad, aget, channel;
            mapped_t value;
            getline(file,line,'\n');
            std::stringstream lineStream(line);
            std::string element;

            getline(lineStream, element,',');
            if (element == "-1" || element == "") continue; // KLUDGE!
            cobo = static_cast<addr_t>(stoi(element));

            getline(lineStream, element,',');
            asad = static_cast<addr_t>(stoi(element));

            getline(lineStream, element,',');
            aget = static_cast<addr_t>(stoi(element));

            getline(lineStream, element,',');
            channel = static_cast<addr_t>(stoi(element));

            auto hash = CalculateHash(cobo, asad, aget, channel);

            getline(lineStream, element);
            value = static_cast<mapped_t>(stoi(element));

            table.emplace(hash, value);
        }

    }

    //! \brief The value returned when a pad is missing from the lookup table.
    mapped_t missingValue {missingValue_};

protected:
    /** \brief Hashing function for the hashtable used to store the values.

      It simply concatenates cobo+asad+aget+channel since this is unique for each value.
      */
    static hash_t CalculateHash(addr_t cobo, addr_t asad, addr_t aget, addr_t channel)
    {
        // Widen the integers so they don't overflow on multiplication
        auto wcobo = hash_t(cobo);
        auto wasad = hash_t(asad);
        auto waget = hash_t(aget);
        auto wchannel = hash_t(channel);

        auto result = wchannel + waget*100 + wasad*10000 + wcobo*1000000;

        return result;
    }

    std::unordered_map<hash_t,mapped_t> table;  // The hashtable, maps hash:value

    friend class EventTestFixture;
    friend class PadLookupTableTestFixture;
};

#endif /* defined(LOOKUPTABLE_H) */
