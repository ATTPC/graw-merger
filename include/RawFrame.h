#ifndef RAWFRAME_H
#define RAWFRAME_H

#include <memory>

class RawFrame
{
public:
    RawFrame() : data_size(0) {}
    RawFrame(const size_t size) : data(new uint8_t[size]), data_size(size) {}
    RawFrame(RawFrame&&) = default;

    typedef uint8_t* iterator;
    typedef const uint8_t* const_iterator;

    iterator begin() { return data.get(); }
    iterator end() { return data.get() + data_size; }
    const_iterator begin() const { return data.get(); }
    const_iterator end() const { return data.get() + data_size; }

    uint8_t* getRawPointer() { return data.get(); }

    size_t size() const { return data_size; }

private:
    std::unique_ptr<uint8_t> data;
    const size_t data_size;
};

#endif /* end of include guard: RAWFRAME_H */
