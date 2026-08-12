#pragma once

#include "ESPressio_IClock.hpp"

namespace ESPressio {

    namespace Timing {

        class IRTCClock : public virtual IClockSettable {
            public:
            // Methods

                virtual bool Synchronize() = 0;
                virtual void OnRTCInterrupt() = 0;
                virtual void OnRTCInterrupt(ClockTime time) = 0;

            // Getters

                virtual bool GetIsSynchronized() const = 0;
        };

    }

}
