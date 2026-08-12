#pragma once

#include <mutex>

namespace ESPressio {

    namespace Timing {

        struct ThreadSafeLockPolicy {
            typedef std::mutex Mutex;
            typedef std::lock_guard<std::mutex> Guard;
        };

    }

}
