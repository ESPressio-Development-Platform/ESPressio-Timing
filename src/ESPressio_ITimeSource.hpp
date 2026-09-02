#pragma once

#include <cstdint>

namespace ESPressio {

    namespace Timing {

        /// <summary>Raw monotonic tick source used by clock implementations independently of their public time representation.</summary>
        class ITimeSource {
            public:
                virtual ~ITimeSource() = default;

                /// <summary>Returns the current monotonically increasing source tick count.</summary>
                virtual uint64_t GetTicks() const = 0;
                /// <summary>Returns the number of source ticks occurring per second.</summary>
                virtual uint64_t GetTicksPerSecond() const = 0;
        };

    }

}
