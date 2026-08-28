#pragma once

#include <mutex>

namespace ESPressio {

    namespace Timing {

        /// <summary>Standard mutex-backed lock policy for clock implementations shared across threads.</summary>
        struct ThreadSafeLockPolicy {
            /// <summary>Mutex type used to protect clock state.</summary>
            using Mutex = std::mutex;
            /// <summary>RAII guard type used to acquire the policy mutex.</summary>
            using Guard =
                std::lock_guard<std::mutex>;
        };

    }

}
