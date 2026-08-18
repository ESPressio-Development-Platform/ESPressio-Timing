#pragma once

#include <cstdint>

#include "ESPressio_GPTimerTimeSource.hpp"
#include "ESPressio_IStopwatchClock.hpp"
#include "ESPressio_StopwatchClock.hpp"

#if ESPRESSIO_TIMING_HAS_GPTIMER

namespace ESPressio {

    namespace Timing {

        template<
            typename TTime = DefaultClockTime,
            typename TLockPolicy =
                ThreadSafeLockPolicy,
            typename TTick = ClockTick
        >
        class GPTimerClock :
            public IStopwatchClock<
                TTime
            > {

            private:
                GPTimerTimeSource _timeSource;

                StopwatchClock<
                    TTime,
                    TLockPolicy,
                    TTick
                > _stopwatch;


            public:
                using TimeType = TTime;
                using TickType = TTick;


                explicit GPTimerClock(
                    bool startImmediately = false,
                    uint32_t requestedResolution =
                        ESPRESSIO_TIMING_GPTIMER_DEFAULT_RESOLUTION_HZ
                )
                    : _timeSource(
                        requestedResolution
                    ),
                      _stopwatch(
                          startImmediately &&
                              _timeSource.
                                  GetIsAvailable(),
                          &_timeSource
                      ) {
                }


                GPTimerClock(
                    const GPTimerClock&
                ) = delete;

                GPTimerClock& operator=(
                    const GPTimerClock&
                ) = delete;

                GPTimerClock(
                    GPTimerClock&&
                ) = delete;

                GPTimerClock& operator=(
                    GPTimerClock&&
                ) = delete;


                void Start() override {
                    if (
                        _timeSource.
                            GetIsAvailable()
                    ) {
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
                    if (
                        _timeSource.
                            GetIsAvailable()
                    ) {
                        _stopwatch.Restart();
                    }
                }


                TTime GetTime() const override {
                    return
                        _stopwatch.GetTime();
                }


                TTime GetResolution() const override {
                    return
                        _stopwatch.GetResolution();
                }


                TTime GetLapTime() const override {
                    return
                        _stopwatch.GetLapTime();
                }


                bool GetIsRunning() const override {
                    return
                        _timeSource.
                            GetIsAvailable() &&
                        _stopwatch.
                            GetIsRunning();
                }


                bool GetIsAvailable() const {
                    return
                        _timeSource.
                            GetIsAvailable();
                }


                esp_err_t
                GetInitializationResult() const {
                    return
                        _timeSource.
                            GetInitializationResult();
                }


                Observable::ObserverHandlePtr
                RegisterObserver(
                    IStopwatchClockObserver<
                        TTime,
                        TTick
                    >* observer
                ) {
                    return
                        _stopwatch.RegisterObserver(
                            observer
                        );
                }


                void UnregisterObserver(
                    IStopwatchClockObserver<
                        TTime,
                        TTick
                    >* observer
                ) {
                    _stopwatch.UnregisterObserver(
                        observer
                    );
                }


                ITimeSource*
                GetTimeSource() {
                    return &_timeSource;
                }


                void SetTime(
                    const TTime& time
                ) override {
                    _stopwatch.SetTime(
                        time
                    );
                }
        };


        template<
            typename TTime = DefaultClockTime,
            typename TTick = ClockTick
        >
        using SingleThreadedGPTimerClock =
            GPTimerClock<
                TTime,
                NoLockPolicy,
                TTick
            >;

    }

}

#endif
