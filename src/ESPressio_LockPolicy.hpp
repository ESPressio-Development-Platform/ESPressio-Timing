#pragma once

namespace ESPressio {

    namespace Timing {

        struct NoLockPolicy {
            struct Mutex { };

            class Guard {
                public:
                    explicit Guard(Mutex&) { }
            };
        };

    }

}
