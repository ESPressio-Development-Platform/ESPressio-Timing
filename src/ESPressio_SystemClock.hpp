#pragma once

#include <cstddef>
#include <utility>

#include "ESPressio_Clock.hpp"
#include "ESPressio_ISystemClock.hpp"
#include "ESPressio_LockPolicy.hpp"
#include "ESPressio_ThreadSafeLockPolicy.hpp"

#ifndef ESPRESSIO_TIMING_MAX_CALLBACKS
    #define ESPRESSIO_TIMING_MAX_CALLBACKS 8
#endif

namespace ESPressio {

    namespace Timing {

        template <typename TLockPolicy>
        class BasicSystemClock :
            public BasicClockSettableBase<TLockPolicy>,
            public ISystemClock {
            private:
                struct ScheduledCallback {
                    ClockTick time = 0;
                    ClockCallback callback = nullptr;
                };

                ScheduledCallback
                    _callbacks[ESPRESSIO_TIMING_MAX_CALLBACKS];
                mutable typename TLockPolicy::Mutex _callbacksMutex;

                explicit BasicSystemClock(
                    ITimeSource* timeSource =
                        BasicHighResolutionTimeSource<
                            TLockPolicy
                        >::GetInstance()
                ) : BasicClockSettableBase<TLockPolicy>(timeSource) { }

            public:
            // Deleted Copy/Move

                BasicSystemClock(const BasicSystemClock&) = delete;
                BasicSystemClock& operator=(const BasicSystemClock&) = delete;
                BasicSystemClock(BasicSystemClock&&) = delete;
                BasicSystemClock& operator=(BasicSystemClock&&) = delete;

            // Static Methods

                static BasicSystemClock* GetInstance(
                    ITimeSource* timeSource =
                        BasicHighResolutionTimeSource<
                            TLockPolicy
                        >::GetInstance()
                ) {
                    static BasicSystemClock instance(timeSource);
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

                    typename TLockPolicy::Guard lock(_callbacksMutex);

                    for (
                        std::size_t index = 0;
                        index < ESPRESSIO_TIMING_MAX_CALLBACKS;
                        ++index
                    ) {
                        if (!_callbacks[index].callback) {
                            _callbacks[index].time =
                                ClockBase::GetNanoseconds(time);
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
                    const ClockTick currentTime =
                        ClockBase::GetNanoseconds(this->GetTime());
                    ClockCallback callbacks[ESPRESSIO_TIMING_MAX_CALLBACKS];

                    {
                        typename TLockPolicy::Guard lock(_callbacksMutex);

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
                    typename TLockPolicy::Guard lock(_callbacksMutex);
                    for (
                        std::size_t index = 0;
                        index < ESPRESSIO_TIMING_MAX_CALLBACKS;
                        ++index
                    ) {
                        _callbacks[index] = ScheduledCallback();
                    }
                }
        };

        typedef BasicSystemClock<ThreadSafeLockPolicy> SystemClock;
        typedef BasicSystemClock<NoLockPolicy> SingleThreadedSystemClock;

    }

}
