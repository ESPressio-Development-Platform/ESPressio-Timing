#pragma once

#include <cstdint>

#include "ESPressio_GPTimerTimeSource.hpp"
#include "ESPressio_IStopwatchClock.hpp"
#include "ESPressio_StopwatchClock.hpp"

#if ESPRESSIO_TIMING_HAS_GPTIMER

namespace ESPressio {

    namespace Timing {

        class GPTimerClock : public IStopwatchClock {
            private:
                GPTimerTimeSource _timeSource;
                StopwatchClock _stopwatch;

            public:
            // Constructor

                explicit GPTimerClock(
                    bool startImmediately = false,
                    uint32_t requestedResolution =
                        ESPRESSIO_TIMING_GPTIMER_DEFAULT_RESOLUTION_HZ
                ) : _timeSource(requestedResolution),
                    _stopwatch(
                        startImmediately && _timeSource.GetIsAvailable(),
                        &_timeSource
                    ) { }

            // Deleted Copy/Move

                GPTimerClock(const GPTimerClock&) = delete;
                GPTimerClock& operator=(const GPTimerClock&) = delete;
                GPTimerClock(GPTimerClock&&) = delete;
                GPTimerClock& operator=(GPTimerClock&&) = delete;

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

    }

}

#endif
