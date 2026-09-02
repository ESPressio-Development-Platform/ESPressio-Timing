#pragma once

namespace ESPressio {

    namespace Timing {

        /// <summary>No-op lock policy for clock implementations whose callers guarantee single-threaded access.</summary>
        struct NoLockPolicy {
            /// <summary>Placeholder mutex type requiring no storage or synchronization.</summary>
            struct Mutex {
            };

            /// <summary>No-op scoped guard matching the lock-policy interface.</summary>
            class Guard {
                public:
                    explicit Guard(
                        Mutex&
                    ) {
                    }
            };
        };

    }

}
