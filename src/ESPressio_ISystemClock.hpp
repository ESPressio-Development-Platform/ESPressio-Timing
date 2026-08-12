#pragma once

#include <functional>

#include "ESPressio_IClock.hpp"

namespace ESPressio {

    namespace Timing {

        class ISystemClock : public virtual IClockSettable {
            public:
            // Type Defs

                typedef std::function<void()> ClockCallback;

            // Methods

                virtual void SetCallback(
                    ClockTime time,
                    ClockCallback callback
                ) = 0;

                virtual void Update() = 0;
                virtual void ClearCallbacks() = 0;
        };

    }

}
