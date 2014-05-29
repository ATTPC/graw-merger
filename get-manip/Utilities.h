//
//  Utilities.h
//  get-manip
//
//  Created by Joshua Bradt on 5/15/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __get_manip__Utilities_h
#define __get_manip__Utilities_h

#include <vector>

namespace Utilities {
    
    template<typename outType>
    outType ExtractInt(std::vector<uint8_t>::const_iterator begin,
                       std::vector<uint8_t>::const_iterator end)
    {
        outType result = 0;
        outType n = 0;
        for (auto iter = begin; iter != end; iter++) {
            
            // WARNING: This needs to be this explicit and step-by-step
            // in order to prevent the computer from assuming the values are
            // signed and filling in the more-significant bits with 1's.
            
            outType val = (*iter);
            val <<= (8*n);
            result |= val;
            n++;
        }
        return result;
    }
    
    template<typename outType>
    outType ExtractByteSwappedInt(std::vector<uint8_t>::const_iterator begin,
                                  std::vector<uint8_t>::const_iterator end)
    {
        outType result = 0;
        outType n = 0;
        for (auto iter = end-1; iter >= begin; iter--) {
            
            // WARNING: This needs to be this explicit and step-by-step
            // in order to prevent the computer from assuming the values are
            // signed and filling in the more-significant bits with 1's.
            
            outType val = (*iter);
            val <<= (8*n);
            result |= val;
            n++;
        }
        return result;
    }
    
}

#endif
