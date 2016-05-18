#ifndef __get_manip__HARDWARE_ADDRESS__
#define __get_manip__HARDWARE_ADDRESS__

#include "Constants.h"

struct HardwareAddress
{
    addr_t cobo;
    addr_t asad;
    addr_t aget;
    addr_t channel;
    addr_t pad;
};

#endif /* end of include guard: __get_manip__HARDWARE_ADDRESS__ */
