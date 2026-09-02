#pragma once

#include <cstdint>
#include <limits>

#include <ESPressio_SystemPlatformClock.hpp>

#include "ESPressio_ClockTypes.hpp"
#include "ESPressio_ITimeSource.hpp"
#include "ESPressio_GPTimerTimeSource.hpp"
#include "ESPressio_LockPolicy.hpp"
#include "ESPressio_ThreadSafeLockPolicy.hpp"

#ifndef ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT
    #define ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT 1
#endif

namespace ESPressio {
namespace Timing {

    namespace Internal {

        /// <summary>Converts raw source ticks to saturating nanoseconds using the source frequency.</summary>
        inline ClockTick TicksToNanoseconds(
            uint64_t ticks,
            uint64_t ticksPerSecond
        ) {
            if (ticksPerSecond == 0) return 0;

            const uint64_t wholeSeconds = ticks / ticksPerSecond;
            const uint64_t remainingTicks = ticks % ticksPerSecond;
            const uint64_t maximum = std::numeric_limits<uint64_t>::max();

            if (wholeSeconds > maximum / NanosecondsPerSecond) return maximum;

            const uint64_t wholeNanoseconds = wholeSeconds * NanosecondsPerSecond;
            const uint64_t remainingNanoseconds =
                remainingTicks <= maximum / NanosecondsPerSecond
                    ? (remainingTicks * NanosecondsPerSecond) / ticksPerSecond
                    : static_cast<uint64_t>(
                        (static_cast<long double>(remainingTicks) * NanosecondsPerSecond) /
                        ticksPerSecond
                      );

            if (remainingNanoseconds > maximum - wholeNanoseconds) return maximum;
            return wholeNanoseconds + remainingNanoseconds;
        }

        /// <summary>Returns the nanosecond resolution corresponding to a raw tick frequency.</summary>
        inline ClockTick GetSourceResolution(uint64_t ticksPerSecond) {
            if (ticksPerSecond == 0) return 0;
            return ticksPerSecond >= NanosecondsPerSecond
                ? 1
                : (NanosecondsPerSecond + ticksPerSecond - 1) / ticksPerSecond;
        }

    }

    /// <summary>Default monotonic time source preferring the configured high-resolution counter and falling back to the System platform clock.</summary>
    template<typename TLockPolicy>
    class HighResolutionTimeSourceT : public ITimeSource {
    private:
#if ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT
        GPTimerTimeSource _highResolutionCounter;
#endif

    public:
        /// <inheritdoc/>
        uint64_t GetTicks() const override {
#if ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT
            if (_highResolutionCounter.GetIsAvailable()) {
                return _highResolutionCounter.GetTicks();
            }
#endif
            return System::Clock::Monotonic().NowNanoseconds();
        }

        /// <inheritdoc/>
        uint64_t GetTicksPerSecond() const override {
#if ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT
            if (_highResolutionCounter.GetIsAvailable()) {
                return _highResolutionCounter.GetTicksPerSecond();
            }
#endif
            return NanosecondsPerSecond;
        }

        /// <summary>Indicates whether the preferred high-resolution platform counter is active.</summary>
        bool GetIsUsingHighResolutionCounter() const {
#if ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT
            return _highResolutionCounter.GetIsAvailable();
#else
            return false;
#endif
        }

        /// <summary>Compatibility alias for <c>GetIsUsingHighResolutionCounter()</c>.</summary>
        bool GetIsUsingGPTimer() const {
            return GetIsUsingHighResolutionCounter();
        }

        /// <summary>Returns the process-wide singleton instance for this lock-policy specialization.</summary>
        static HighResolutionTimeSourceT* GetInstance() {
            static HighResolutionTimeSourceT instance;
            return &instance;
        }
    };

    /// <summary>Thread-safe default high-resolution time source.</summary>
    using HighResolutionTimeSource =
        HighResolutionTimeSourceT<ThreadSafeLockPolicy>;

    /// <summary>Single-threaded high-resolution time-source specialization.</summary>
    using SingleThreadedHighResolutionTimeSource =
        HighResolutionTimeSourceT<NoLockPolicy>;

}
}
