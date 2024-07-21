#pragma once

#include <cstdint>
#include <functional>

namespace ESPressio {

    namespace Timing {

        /*
            `IClock` is a common Interface for all Clock Types provided by this library.
            You can use it to reference any Clock Type without knowing the actual type.
        */
        class IClock {
            public:

            // Methods

                /// `GetTime` returns the current time in the Clock's Unsigned Base Type.
                virtual unsigned int GetTime() = 0;
        };

        /*
            `IClockSettable` is a common Interface for all Clock Types that can be set.
            You can use it to reference any Settable Clock Type without knowing the actual type.
        */
        class IClockSettable {
            public:

            // Methods

                /// `SetTime` sets the current time in the Clock's Unsigned Base Type.
                virtual void SetTime(unsigned int time) = 0;
        };

    }

}