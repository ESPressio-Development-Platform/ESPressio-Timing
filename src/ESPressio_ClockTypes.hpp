#pragma once

#include <cstdint>
#include <ESPressio_Time.hpp>

namespace ESPressio {

    namespace Timing {

        /*
         * Raw internal clock storage.
         *
         * Public time representations are deliberately separate from raw
         * monotonic storage. Clock algorithms therefore do not acquire any
         * serialization or presentation overhead merely because TTime does.
         */
        using ClockTick = uint64_t;

        static constexpr ClockTick NanosecondsPerMicrosecond = 1000ULL;
        static constexpr ClockTick NanosecondsPerMillisecond = 1000000ULL;
        static constexpr ClockTick NanosecondsPerSecond = 1000000000ULL;

        /*
         * Default public representation. Timing itself depends only on the
         * ordinary ESPressio Units Time type.
         *
         * Applications may select any compatible TTime, including an optional
         * SerializableTime type supplied by ESPressio Units.
         */
        using DefaultClockTime =
            Units::Time<
                uint64_t,
                Units::Nano
            >;

    }

}
