#pragma once

#include <cstdint>

namespace ESPressio {

    namespace Timing {

        /*
         * Raw monotonic hardware/framework source.
         *
         * This remains completely independent of the public TTime
         * representation.
         */
        class ITimeSource {
            public:
                virtual ~ITimeSource() = default;

                virtual uint64_t GetTicks() const = 0;
                virtual uint64_t GetTicksPerSecond() const = 0;
        };

    }

}
