#pragma once

#include <functional>

#include "ESPressio_IClock.hpp"

namespace ESPressio {

    namespace Timing {

        template<typename TTime = DefaultClockTime>
        class ISystemClock :
            public virtual IClockSettable<TTime> {

            public:
                using TimeType = TTime;
                using ClockCallback =
                    std::function<void()>;

                virtual void SetCallback(
                    const TTime& time,
                    ClockCallback callback
                ) = 0;

                virtual void Update() = 0;
                virtual void ClearCallbacks() = 0;
        };

    }

}
