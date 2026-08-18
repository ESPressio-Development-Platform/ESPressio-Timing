#pragma once

#include <mutex>

namespace ESPressio {

    namespace Timing {

        struct ThreadSafeLockPolicy {
            using Mutex = std::mutex;
            using Guard =
                std::lock_guard<std::mutex>;
        };

    }

}
