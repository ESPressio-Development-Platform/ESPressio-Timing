#pragma once

#include <functional>

#include "ESPressio_IClock.hpp"

namespace ESPressio {

    namespace Timing {

        /// <summary>Settable system clock capable of scheduling callbacks against clock time.</summary>
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members: none; polymorphic interface/object includes vptr storage where not supplied by a base.
 * Total Memory: 4 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
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
