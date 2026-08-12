#pragma once

#include <cstdint>

#include <ESPressio_Time.hpp>

namespace ESPressio {

    namespace Timing {

        typedef uint64_t ClockTick;
        typedef Units::Time<uint64_t, Units::Nano> ClockTime;

        static constexpr ClockTick NanosecondsPerMicrosecond = 1000ULL;
        static constexpr ClockTick NanosecondsPerMillisecond = 1000000ULL;
        static constexpr ClockTick NanosecondsPerSecond = 1000000000ULL;

        /*
            `IClock` is the common interface for every clock in this library.
            Values use the ESPressio Units `Time` context. Their runtime order
            of magnitude represents the clock's actual precision.
        */
        class IClock {
            public:
            // Destructor

                virtual ~IClock() { }

            // Getters

                virtual ClockTime GetTime() const = 0;
                virtual ClockTime GetResolution() const = 0;
        };

        class IClockSettable : public virtual IClock {
            public:
            // Setters

                virtual void SetTime(ClockTime time) = 0;
        };

    }

}
