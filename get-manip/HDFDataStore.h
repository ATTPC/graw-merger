#ifndef HDFDATASTORE_H
#define HDFDATASTORE_H

#include <string>
#include <H5Cpp.h>
#include "Event.h"
#include "Constants.h"

class HDFDataStore
{
public:
    HDFDataStore(const std::string& filename, const bool writable=false);

    void writeEvent(const Event& evt);


private:
    H5::H5File file;
    H5::Group gp;
    std::string groupName = "get";
};

#endif /* end of include guard: HDFDATASTORE_H */
