#pragma once

#include <cstdint>

namespace ESPressio {

    namespace Timing {

        /*
            A raw monotonic hardware or framework time source. Keeping ticks
            and frequency separate preserves the source's native precision.
        */
        class ITimeSource {
            public:
            // Destructor

                virtual ~ITimeSource() { }

            // Getters

                virtual uint64_t GetTicks() const = 0;
                virtual uint64_t GetTicksPerSecond() const = 0;
        };

    }

}
