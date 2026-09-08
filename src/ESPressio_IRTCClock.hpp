#pragma once

#include "ESPressio_IClock.hpp"

namespace ESPressio {

    namespace Timing {

        /// <summary>Settable clock contract synchronized from a real-time clock source and optional RTC interrupts.</summary>
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members: none; polymorphic interface/object includes vptr storage where not supplied by a base.
 * Total Memory: 4 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
template<typename TTime = DefaultClockTime>
        class IRTCClock :
            public virtual IClockSettable<TTime> {

            public:
                using TimeType = TTime;

                /// <summary>Attempts to synchronize the clock from its configured RTC source.</summary>
                virtual bool Synchronize() = 0;
                /// <summary>Notifies the clock that an RTC interrupt occurred and the current RTC value should be acquired.</summary>
                virtual void OnRTCInterrupt() = 0;

                /// <summary>Notifies the clock of an RTC interrupt with the timestamp already captured by the caller.</summary>
                virtual void OnRTCInterrupt(
                    const TTime& time
                ) = 0;

                /// <summary>Indicates whether the clock currently has a valid RTC synchronization.</summary>
                virtual bool GetIsSynchronized() const = 0;
        };

    }

}
