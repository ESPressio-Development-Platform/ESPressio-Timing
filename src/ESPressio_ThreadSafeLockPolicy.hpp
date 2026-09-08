#pragma once

#include <mutex>

namespace ESPressio {

    namespace Timing {

        /// <summary>Standard mutex-backed lock policy for clock implementations shared across threads.</summary>
/**
 * ESPressio Memory Audit
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 0 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
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
