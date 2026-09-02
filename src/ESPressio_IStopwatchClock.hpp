#pragma once

#include "ESPressio_IClock.hpp"

namespace ESPressio {

    namespace Timing {

        /// <summary>Settable elapsed-time clock supporting stopwatch lifecycle and lap-time queries.</summary>
        template<typename TTime = DefaultClockTime>
        class IStopwatchClock :
            public virtual IClockSettable<TTime> {

            public:
                using TimeType = TTime;

                /// <summary>Starts or resumes elapsed-time accumulation.</summary>
                virtual void Start() = 0;
                /// <summary>Stops elapsed-time accumulation while preserving the current elapsed value.</summary>
                virtual void Stop() = 0;
                /// <summary>Resets the elapsed value to the implementation-defined origin.</summary>
                virtual void Reset() = 0;
                /// <summary>Resets the elapsed value and starts timing again.</summary>
                virtual void Restart() = 0;

                /// <summary>Indicates whether elapsed-time accumulation is currently running.</summary>
                virtual bool GetIsRunning() const = 0;
                /// <summary>Returns the elapsed time captured for the current lap without changing running state.</summary>
                virtual TTime GetLapTime() const = 0;
        };

    }

}
