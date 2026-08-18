#pragma once

#include "ESPressio_IClock.hpp"

namespace ESPressio {

    namespace Timing {

        template<typename TTime = DefaultClockTime>
        class IStopwatchClock :
            public virtual IClockSettable<TTime> {

            public:
                using TimeType = TTime;

                virtual void Start() = 0;
                virtual void Stop() = 0;
                virtual void Reset() = 0;
                virtual void Restart() = 0;

                virtual bool GetIsRunning() const = 0;
                virtual TTime GetLapTime() const = 0;
        };

    }

}
