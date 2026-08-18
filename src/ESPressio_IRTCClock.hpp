#pragma once

#include "ESPressio_IClock.hpp"

namespace ESPressio {

    namespace Timing {

        template<typename TTime = DefaultClockTime>
        class IRTCClock :
            public virtual IClockSettable<TTime> {

            public:
                using TimeType = TTime;

                virtual bool Synchronize() = 0;
                virtual void OnRTCInterrupt() = 0;

                virtual void OnRTCInterrupt(
                    const TTime& time
                ) = 0;

                virtual bool GetIsSynchronized() const = 0;
        };

    }

}
