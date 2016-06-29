#ifndef UTILITIES_H
#define UTILITIES_H

#include <vector>
#include <string>
#include <limits>
#include <exception>

class BadCast : public std::exception
{
public:
    BadCast(const std::string& s) : reason(s) {}
    const char* what() const noexcept { return reason.c_str(); }

private:
    std::string reason;
};

template <typename toType, typename fromType>
toType narrow_cast(fromType value)
{
    if (value < std::numeric_limits<toType>::max()) {
        return static_cast<toType>(value);
    }
    else {
        throw BadCast("Casting failed: input number was too large.");
    }
}

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
