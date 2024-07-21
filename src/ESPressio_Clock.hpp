#pragma once

#include "ESPressio_IClock.hpp"

#ifndef ESPRESSIO_TIMING_DEFAULT_PRECISION
    #define ESPRESSIO_TIMING_DEFAULT_PRECISION 1 * 1000 * 1000 // 1MHz (1us precision)
#endif

namespace ESPressio {

    namespace Timing {

        class ClockBase : public IClock {

        };

        class ClockSettableBase : public ClockBase, IClockSettable {

        };

    }

}