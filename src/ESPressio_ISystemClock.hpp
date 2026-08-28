#pragma once

#include <functional>

#include "ESPressio_IClock.hpp"

namespace ESPressio {

    namespace Timing {

        /// <summary>Settable system clock capable of scheduling callbacks against clock time.</summary>
        template<typename TTime = DefaultClockTime>
        class ISystemClock :
            public virtual IClockSettable<TTime> {

            public:
                using TimeType = TTime;
                /// <summary>Callable invoked when a scheduled system-clock time is reached.</summary>
                using ClockCallback =
                    std::function<void()>;

                /// <summary>Schedules a callback for the supplied clock time.</summary>
                virtual void SetCallback(
                    const TTime& time,
                    ClockCallback callback
                ) = 0;

                /// <summary>Processes due callbacks and implementation-specific clock maintenance.</summary>
                virtual void Update() = 0;
                /// <summary>Removes all pending scheduled callbacks.</summary>
                virtual void ClearCallbacks() = 0;
        };

    }

}
