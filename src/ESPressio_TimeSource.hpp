#pragma once

#include <cstdint>
#include <limits>

#include "ESPressio_IClock.hpp"
#include "ESPressio_ITimeSource.hpp"
#include "ESPressio_GPTimerTimeSource.hpp"
#include "ESPressio_LockPolicy.hpp"
#include "ESPressio_ThreadSafeLockPolicy.hpp"

#ifndef ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT
    #define ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT 1
#endif

#if defined(ESP32)
    #include <esp_timer.h>
#elif defined(ARDUINO)
    #include <Arduino.h>
#else
    #include <chrono>
#endif

namespace ESPressio {

    namespace Timing {

        namespace Internal {

            inline ClockTick TicksToNanoseconds(
                uint64_t ticks,
                uint64_t ticksPerSecond
            ) {
                if (ticksPerSecond == 0) {
                    return 0;
                }

                const uint64_t wholeSeconds = ticks / ticksPerSecond;
                const uint64_t remainingTicks = ticks % ticksPerSecond;
                const uint64_t maximum =
                    std::numeric_limits<uint64_t>::max();

                if (wholeSeconds > maximum / NanosecondsPerSecond) {
                    return maximum;
                }

                const uint64_t wholeNanoseconds =
                    wholeSeconds * NanosecondsPerSecond;
                const uint64_t remainingNanoseconds =
                    remainingTicks <= maximum / NanosecondsPerSecond
                        ? (remainingTicks * NanosecondsPerSecond) /
                            ticksPerSecond
                        : static_cast<uint64_t>(
                            (static_cast<long double>(remainingTicks) *
                                NanosecondsPerSecond) /
                            ticksPerSecond
                        );

                if (remainingNanoseconds > maximum - wholeNanoseconds) {
                    return maximum;
                }

                return wholeNanoseconds + remainingNanoseconds;
            }

            inline ClockTick GetSourceResolution(
                uint64_t ticksPerSecond
            ) {
                if (ticksPerSecond == 0) {
                    return 0;
                }

                return ticksPerSecond >= NanosecondsPerSecond
                    ? 1
                    : (NanosecondsPerSecond + ticksPerSecond - 1) /
                        ticksPerSecond;
            }

        }

        template <typename TLockPolicy>
        class BasicHighResolutionTimeSource : public ITimeSource {
            #if ESPRESSIO_TIMING_HAS_GPTIMER && \
                ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT
            private:
                GPTimerTimeSource _gptimerTimeSource;
            #endif

            #if defined(ARDUINO) && !defined(ESP32)
            private:
                mutable uint32_t _lastTick = 0;
                mutable uint64_t _tickEpoch = 0;
                mutable typename TLockPolicy::Mutex _tickMutex;
            #endif

            public:
            // Getters

                uint64_t GetTicks() const override {
                    #if defined(ESP32)
                        #if ESPRESSIO_TIMING_HAS_GPTIMER && \
                            ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT
                            if (_gptimerTimeSource.GetIsAvailable()) {
                                return _gptimerTimeSource.GetTicks();
                            }
                        #endif
                        return static_cast<uint64_t>(esp_timer_get_time());
                    #elif defined(ARDUINO)
                        const uint32_t currentTick = micros();
                        typename TLockPolicy::Guard lock(_tickMutex);

                        static constexpr uint64_t TickRange = 1ULL << 32;
                        static constexpr uint32_t HalfTickRange =
                            1UL << 31;

                        if (currentTick < _lastTick) {
                            const uint32_t difference =
                                _lastTick - currentTick;

                            if (difference > HalfTickRange) {
                                _tickEpoch += TickRange;
                                _lastTick = currentTick;
                            }

                            return _tickEpoch + currentTick;
                        }

                        if (currentTick - _lastTick > HalfTickRange &&
                            _tickEpoch >= TickRange) {
                            return (_tickEpoch - TickRange) + currentTick;
                        }

                        _lastTick = currentTick;
                        return _tickEpoch + currentTick;
                    #else
                        typedef std::chrono::steady_clock SourceClock;
                        return static_cast<uint64_t>(
                            SourceClock::now().time_since_epoch().count()
                        );
                    #endif
                }

                uint64_t GetTicksPerSecond() const override {
                    #if defined(ESP32)
                        #if ESPRESSIO_TIMING_HAS_GPTIMER && \
                            ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT
                            if (_gptimerTimeSource.GetIsAvailable()) {
                                return _gptimerTimeSource.GetTicksPerSecond();
                            }
                        #endif
                        return 1000000ULL;
                    #elif defined(ARDUINO)
                        return 1000000ULL;
                    #else
                        typedef std::chrono::steady_clock SourceClock;
                        return static_cast<uint64_t>(
                            SourceClock::period::den / SourceClock::period::num
                        );
                    #endif
                }

                bool GetIsUsingGPTimer() const {
                    #if ESPRESSIO_TIMING_HAS_GPTIMER && \
                        ESPRESSIO_TIMING_USE_GPTIMER_BY_DEFAULT
                        return _gptimerTimeSource.GetIsAvailable();
                    #else
                        return false;
                    #endif
                }

            // Static Methods

                static BasicHighResolutionTimeSource* GetInstance() {
                    static BasicHighResolutionTimeSource instance;
                    return &instance;
                }
        };

        typedef BasicHighResolutionTimeSource<ThreadSafeLockPolicy>
            HighResolutionTimeSource;
        typedef BasicHighResolutionTimeSource<NoLockPolicy>
            SingleThreadedHighResolutionTimeSource;

    }

}
