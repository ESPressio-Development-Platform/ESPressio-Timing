#pragma once

#include <mutex>

namespace ESPressio {

    namespace Timing {

        /// <summary>Standard mutex-backed lock policy for clock implementations shared across threads.</summary>
/**
 * ESPressio Memory Audit
 * Members: none (standalone empty object occupies 1 byte; an eligible empty base may be optimized to 0 bytes).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
struct ThreadSafeLockPolicy {
            /// <summary>Mutex type used to protect clock state.</summary>
            using Mutex = std::mutex;
            /// <summary>RAII guard type used to acquire the policy mutex.</summary>
            using Guard =
                std::lock_guard<std::mutex>;
        };

    }

}
