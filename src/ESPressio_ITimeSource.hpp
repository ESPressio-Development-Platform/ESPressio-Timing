#pragma once

#include <cstdint>

namespace ESPressio {

    namespace Timing {

        /// <summary>Raw monotonic tick source used by clock implementations independently of their public time representation.</summary>
/**
 * ESPressio Memory Audit
 * Members: none; polymorphic/virtual-base object metadata is included in the total.
 * Total Memory: 4 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
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
