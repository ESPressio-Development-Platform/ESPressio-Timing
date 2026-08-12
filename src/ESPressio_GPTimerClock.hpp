#pragma once

#include <cstdint>

#include "ESPressio_GPTimerTimeSource.hpp"
#include "ESPressio_IStopwatchClock.hpp"
#include "ESPressio_StopwatchClock.hpp"

#if ESPRESSIO_TIMING_HAS_GPTIMER

namespace ESPressio {

    namespace Timing {

        template <typename TLockPolicy>
        class BasicGPTimerClock : public IStopwatchClock {
            private:
                GPTimerTimeSource _timeSource;
                BasicStopwatchClock<TLockPolicy> _stopwatch;

            public:
            // Constructor

                explicit BasicGPTimerClock(
                    bool startImmediately = false,
                    uint32_t requestedResolution =
                        ESPRESSIO_TIMING_GPTIMER_DEFAULT_RESOLUTION_HZ
                ) : _timeSource(requestedResolution),
                    _stopwatch(
                        startImmediately && _timeSource.GetIsAvailable(),
                        &_timeSource
                    ) { }

            // Deleted Copy/Move

                BasicGPTimerClock(const BasicGPTimerClock&) = delete;
                BasicGPTimerClock& operator=(
                    const BasicGPTimerClock&
                ) = delete;
                BasicGPTimerClock(BasicGPTimerClock&&) = delete;
                BasicGPTimerClock& operator=(BasicGPTimerClock&&) = delete;

            // Methods

                void Start() override {
                    if (_timeSource.GetIsAvailable()) {
                        _stopwatch.Start();
                    }
                }

                void Stop() override {
                    _stopwatch.Stop();
                }

                void Reset() override {
                    _stopwatch.Reset();
                }

                void Restart() override {
                    if (_timeSource.GetIsAvailable()) {
                        _stopwatch.Restart();
                    }
                }

            // Getters

                ClockTime GetTime() const override {
                    return _stopwatch.GetTime();
                }

                ClockTime GetResolution() const override {
                    return _stopwatch.GetResolution();
                }

                ClockTime GetLapTime() const override {
                    return _stopwatch.GetLapTime();
                }

                bool GetIsRunning() const override {
                    return _timeSource.GetIsAvailable() &&
                        _stopwatch.GetIsRunning();
                }

                bool GetIsAvailable() const {
                    return _timeSource.GetIsAvailable();
                }

                esp_err_t GetInitializationResult() const {
                    return _timeSource.GetInitializationResult();
                }

                ITimeSource* GetTimeSource() {
                    return &_timeSource;
                }

            // Setters

                void SetTime(ClockTime time) override {
                    _stopwatch.SetTime(time);
                }
        };

        typedef BasicGPTimerClock<ThreadSafeLockPolicy> GPTimerClock;
        typedef BasicGPTimerClock<NoLockPolicy>
            SingleThreadedGPTimerClock;

    }

}

#endif
