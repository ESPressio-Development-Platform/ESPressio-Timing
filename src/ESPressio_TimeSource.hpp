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

        inline ClockTick GetSourceResolution(uint64_t ticksPerSecond) {
            if (ticksPerSecond == 0) return 0;
            return ticksPerSecond >= NanosecondsPerSecond
                ? 1
                : (NanosecondsPerSecond + ticksPerSecond - 1) / ticksPerSecond;
        }

    }

    template<typename TLockPolicy>
    class HighResolutionTimeSourceT : public ITimeSource {
    private:
#if ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT
        GPTimerTimeSource _highResolutionCounter;
#endif

    public:
        uint64_t GetTicks() const override {
#if ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT
            if (_highResolutionCounter.GetIsAvailable()) {
                return _highResolutionCounter.GetTicks();
            }
#endif
            return System::Clock::Monotonic().NowNanoseconds();
        }

        uint64_t GetTicksPerSecond() const override {
#if ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT
            if (_highResolutionCounter.GetIsAvailable()) {
                return _highResolutionCounter.GetTicksPerSecond();
            }
#endif
            return NanosecondsPerSecond;
        }

        bool GetIsUsingHighResolutionCounter() const {
#if ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT
            return _highResolutionCounter.GetIsAvailable();
#else
            return false;
#endif
        }

        // Compatibility name retained for callers from the previous ESP32-
        // specific implementation. It no longer implies an ESP-IDF dependency.
        bool GetIsUsingGPTimer() const {
            return GetIsUsingHighResolutionCounter();
        }

        static HighResolutionTimeSourceT* GetInstance() {
            static HighResolutionTimeSourceT instance;
            return &instance;
        }
    };

    using HighResolutionTimeSource =
        HighResolutionTimeSourceT<ThreadSafeLockPolicy>;

    using SingleThreadedHighResolutionTimeSource =
        HighResolutionTimeSourceT<NoLockPolicy>;

}
}
