#pragma once

#include <cstddef>
#include <mutex>
#include <utility>

#include "ESPressio_Clock.hpp"
#include "ESPressio_ISystemClock.hpp"

#ifndef ESPRESSIO_TIMING_MAX_CALLBACKS
    #define ESPRESSIO_TIMING_MAX_CALLBACKS 8
#endif

namespace ESPressio {

    namespace Timing {

        class SystemClock :
            public ClockSettableBase,
            public ISystemClock {
            private:
                struct ScheduledCallback {
                    ClockTick time = 0;
                    ClockCallback callback = nullptr;
                };

                ScheduledCallback
                    _callbacks[ESPRESSIO_TIMING_MAX_CALLBACKS];
                mutable std::mutex _callbacksMutex;

                explicit SystemClock(
                    ITimeSource* timeSource =
                        HighResolutionTimeSource::GetInstance()
                ) : ClockSettableBase(timeSource) { }

            public:
            // Deleted Copy/Move

                SystemClock(const SystemClock&) = delete;
                SystemClock& operator=(const SystemClock&) = delete;
                SystemClock(SystemClock&&) = delete;
                SystemClock& operator=(SystemClock&&) = delete;

            // Static Methods

                static SystemClock* GetInstance(
                    ITimeSource* timeSource =
                        HighResolutionTimeSource::GetInstance()
                ) {
                    static SystemClock instance(timeSource);
                    return &instance;
                }

            // Methods

                bool TrySetCallback(
                    ClockTime time,
                    ClockCallback callback
                ) {
                    if (!callback) {
                        return false;
                    }

                    std::lock_guard<std::mutex> lock(_callbacksMutex);

                    for (
                        std::size_t index = 0;
                        index < ESPRESSIO_TIMING_MAX_CALLBACKS;
                        ++index
                    ) {
                        if (!_callbacks[index].callback) {
                            _callbacks[index].time = GetNanoseconds(time);
                            _callbacks[index].callback =
                                std::move(callback);
                            return true;
                        }
                    }

                    return false;
                }

                void SetCallback(
                    ClockTime time,
                    ClockCallback callback
                ) override {
                    TrySetCallback(time, std::move(callback));
                }

                void Update() override {
                    const ClockTick currentTime = GetNanoseconds(GetTime());
                    ClockCallback callbacks[ESPRESSIO_TIMING_MAX_CALLBACKS];

                    {
                        std::lock_guard<std::mutex> lock(_callbacksMutex);

                        for (
                            std::size_t index = 0;
                            index < ESPRESSIO_TIMING_MAX_CALLBACKS;
                            ++index
                        ) {
                            if (_callbacks[index].callback &&
                                currentTime >= _callbacks[index].time) {
                                callbacks[index] = std::move(
                                    _callbacks[index].callback
                                );
                                _callbacks[index] = ScheduledCallback();
                            }
                        }
                    }

                    for (
                        std::size_t index = 0;
                        index < ESPRESSIO_TIMING_MAX_CALLBACKS;
                        ++index
                    ) {
                        if (callbacks[index]) {
                            callbacks[index]();
                        }
                    }
                }

                void ClearCallbacks() override {
                    std::lock_guard<std::mutex> lock(_callbacksMutex);
                    for (
                        std::size_t index = 0;
                        index < ESPRESSIO_TIMING_MAX_CALLBACKS;
                        ++index
                    ) {
                        _callbacks[index] = ScheduledCallback();
                    }
                }
        };

    }

}
