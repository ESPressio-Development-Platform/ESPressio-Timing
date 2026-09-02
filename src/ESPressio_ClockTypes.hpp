#pragma once

#include <cstdint>
#include <ESPressio_Time.hpp>

namespace ESPressio {

    namespace Timing {

        /// <summary>Unsigned nanosecond-oriented storage type used internally by clock algorithms.</summary>
        /// <remarks>Raw clock storage is intentionally separate from public unit-aware time representations.</remarks>
        using ClockTick = uint64_t;

        /// <summary>Number of nanoseconds in one microsecond.</summary>
        static constexpr ClockTick NanosecondsPerMicrosecond = 1000ULL;
        /// <summary>Number of nanoseconds in one millisecond.</summary>
        static constexpr ClockTick NanosecondsPerMillisecond = 1000000ULL;
        /// <summary>Number of nanoseconds in one second.</summary>
        static constexpr ClockTick NanosecondsPerSecond = 1000000000ULL;

        /// <summary>Default public clock representation: an unsigned 64-bit ESPressio time value expressed in nanoseconds.</summary>
        /// <remarks>Clock templates may use another compatible unit-aware time representation where required.</remarks>
        using DefaultClockTime =
            Units::Time<
                uint64_t,
                Units::Nano
            >;

    }

}
