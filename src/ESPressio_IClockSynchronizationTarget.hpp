#pragma once

#include "ESPressio_ClockSynchronization.hpp"

namespace ESPressio {

    namespace Timing {

        /*
         * Transport-neutral synchronization endpoint.
         *
         * A radio/network implementation only needs this interface to:
         *   - capture timestamps;
         *   - submit a completed four-timestamp exchange;
         *   - inspect synchronization state;
         *   - configure/reset discipline behavior.
         *
         * No public TTime representation is involved.
         */
        template<typename TTick = ClockTick>
        class IClockSynchronizationTarget {
            public:
                virtual ~IClockSynchronizationTarget() =
                    default;


                virtual TTick
                GetSynchronizationTimestampNanoseconds()
                    const = 0;


                virtual ClockSynchronizationResult<TTick>
                SubmitSynchronizationSample(
                    const ClockSynchronizationSample<TTick>&
                        sample,
                    ClockSynchronizationAdjustmentMode
                        adjustmentMode =
                            ClockSynchronizationAdjustmentMode::
                                SlewOnly
                ) = 0;


                virtual ClockSynchronizationStatus<TTick>
                GetSynchronizationStatus()
                    const = 0;


                virtual void
                ConfigureSynchronization(
                    const ClockSynchronizationConfig&
                        config
                ) = 0;


                virtual ClockSynchronizationConfig
                GetSynchronizationConfig()
                    const = 0;


                virtual void
                ResetSynchronization() = 0;
        };

    }

}
