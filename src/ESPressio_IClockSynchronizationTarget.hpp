#pragma once

#include "ESPressio_ClockSynchronization.hpp"

namespace ESPressio {

    namespace Timing {

        /// <summary>Transport-neutral endpoint for capturing timestamps and disciplining a clock from synchronization exchanges.</summary>
        /// <typeparam name="TTick">Raw timestamp representation used by synchronization calculations.</typeparam>
        /// <remarks>Network and radio integrations can depend on this interface without depending on a public unit-aware clock representation.</remarks>
        template<typename TTick = ClockTick>
        class IClockSynchronizationTarget {
            public:
                virtual ~IClockSynchronizationTarget() =
                    default;


                /// <summary>Captures the current synchronization timestamp in nanoseconds.</summary>
                virtual TTick
                GetSynchronizationTimestampNanoseconds()
                    const = 0;


                /// <summary>Submits one completed four-timestamp synchronization exchange for validation and clock adjustment.</summary>
                virtual ClockSynchronizationResult<TTick>
                SubmitSynchronizationSample(
                    const ClockSynchronizationSample<TTick>&
                        sample,
                    ClockSynchronizationAdjustmentMode
                        adjustmentMode =
                            ClockSynchronizationAdjustmentMode::
                                SlewOnly
                ) = 0;


                /// <summary>Returns the current synchronization and discipline status.</summary>
                virtual ClockSynchronizationStatus<TTick>
                GetSynchronizationStatus()
                    const = 0;


                /// <summary>Applies synchronization filtering and discipline configuration.</summary>
                virtual void
                ConfigureSynchronization(
                    const ClockSynchronizationConfig&
                        config
                ) = 0;


                /// <summary>Returns the active synchronization configuration.</summary>
                virtual ClockSynchronizationConfig
                GetSynchronizationConfig()
                    const = 0;


                /// <summary>Clears accumulated synchronization state and discipline history.</summary>
                virtual void
                ResetSynchronization() = 0;
        };

    }

}
