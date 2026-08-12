#pragma once

#include "ESPressio_IClock.hpp"

namespace ESPressio {

    namespace Timing {

        class IStopwatchClock : public virtual IClockSettable {
            public:
            // Methods

                virtual void Start() = 0;
                virtual void Stop() = 0;
                virtual void Reset() = 0;
                virtual void Restart() = 0;

            // Getters

                virtual bool GetIsRunning() const = 0;
                virtual ClockTime GetLapTime() const = 0;
        };

    }

}
