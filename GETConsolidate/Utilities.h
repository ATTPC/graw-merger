//
//  Utilities.h
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/15/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef GETConsolidate_Utilities_h
#define GETConsolidate_Utilities_h

#include <vector>

namespace Utilities {
    
    template<typename outType>
    outType ExtractInt(std::vector<uint8_t>::const_iterator begin,
                       std::vector<uint8_t>::const_iterator end)
    {
        outType result = 0;
        int n = 0;
        for (auto iter = begin; iter != end; iter++) {
            result |= (*iter)<<(8*n);
            n++;
        }
        return result;
    }
    
    template<typename outType>
    outType ExtractByteSwappedInt(std::vector<uint8_t>::iterator begin,
                                  std::vector<uint8_t>::iterator end)
    {
        outType result = 0;
        int n = 0;
        for (auto iter = end-1; iter >= begin; iter--) {
            result |= (*iter)<<(8*n);
            n++;
        }
        return result;
    }
    
}

#endif
