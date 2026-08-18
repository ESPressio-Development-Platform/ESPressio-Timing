#pragma once

#include <cstddef>
#include <functional>
#include <limits>
#include <utility>

#include "ESPressio_ClockTypes.hpp"
#include "ESPressio_ISystemClock.hpp"
#include "ESPressio_TimeSource.hpp"
#include "ESPressio_TimeTraits.hpp"
#include "ESPressio_LockPolicy.hpp"
#include "ESPressio_ThreadSafeLockPolicy.hpp"

#ifndef ESPRESSIO_TIMING_MAX_CALLBACKS
    #define ESPRESSIO_TIMING_MAX_CALLBACKS 8
#endif

namespace ESPressio {

    namespace Timing {

        /*
         * Non-templated singleton state for the system clock.
         *
         * There is exactly one system timeline, regardless of how many typed
         * SystemClock<TTime> facades are instantiated.
         *
         * Public Unit/Serializable representations never live here; the core
         * stores only raw nanosecond ticks and the global callback scheduler.
         */
        template<
            typename TLockPolicy = ThreadSafeLockPolicy,
            typename TTick = ClockTick
        >
        class SystemClockCore {
            private:
                using ClockCallback =
                    std::function<void()>;

                struct ScheduledCallback {
                    TTick Time = 0;
                    ClockCallback Callback = nullptr;
                };

                ITimeSource* _timeSource;

                mutable
                    typename TLockPolicy::Mutex
                        _clockMutex;

                mutable
                    typename TLockPolicy::Mutex
                        _callbacksMutex;

                TTick _baseTime = 0;
                TTick _baseSourceTime = 0;

                ScheduledCallback
                    _callbacks[
                        ESPRESSIO_TIMING_MAX_CALLBACKS
                    ];


                explicit SystemClockCore(
                    ITimeSource* timeSource =
                        HighResolutionTimeSourceT<
                            TLockPolicy
                        >::GetInstance()
                )
                    : _timeSource(
                        timeSource == nullptr
                            ? HighResolutionTimeSourceT<
                                TLockPolicy
                              >::GetInstance()
                            : timeSource
                    ),
                      _baseSourceTime(
                          GetSourceTime()
                      ) {
                }


                TTick GetSourceTime() const {
                    return static_cast<TTick>(
                        Internal::TicksToNanoseconds(
                            _timeSource->GetTicks(),
                            _timeSource->
                                GetTicksPerSecond()
                        )
                    );
                }


                static TTick AddSaturated(
                    TTick left,
                    TTick right
                ) {
                    const TTick maximum =
                        std::numeric_limits<
                            TTick
                        >::max();

                    return
                        right >
                            maximum -
                            left
                            ? maximum
                            : left +
                                right;
                }


            public:
                using TickType = TTick;


                SystemClockCore(
                    const SystemClockCore&
                ) = delete;

                SystemClockCore& operator=(
                    const SystemClockCore&
                ) = delete;

                SystemClockCore(
                    SystemClockCore&&
                ) = delete;

                SystemClockCore& operator=(
                    SystemClockCore&&
                ) = delete;


                static SystemClockCore&
                GetInstance(
                    ITimeSource* timeSource =
                        HighResolutionTimeSourceT<
                            TLockPolicy
                        >::GetInstance()
                ) {
                    static SystemClockCore
                        instance(
                            timeSource
                        );

                    return instance;
                }


                TTick GetTimeNanoseconds() const {
                    const TTick sourceTime =
                        GetSourceTime();

                    typename TLockPolicy::Guard
                        lock(_clockMutex);

                    const TTick elapsed =
                        sourceTime >=
                            _baseSourceTime
                            ? sourceTime -
                                _baseSourceTime
                            : 0;

                    return AddSaturated(
                        _baseTime,
                        elapsed
                    );
                }


                TTick GetResolutionNanoseconds() const {
                    return static_cast<TTick>(
                        Internal::
                            GetSourceResolution(
                                _timeSource->
                                    GetTicksPerSecond()
                            )
                    );
                }


                void SetTimeNanoseconds(
                    TTick time
                ) {
                    const TTick sourceTime =
                        GetSourceTime();

                    typename TLockPolicy::Guard
                        lock(_clockMutex);

                    _baseTime =
                        time;

                    _baseSourceTime =
                        sourceTime;
                }


                bool TrySetCallbackNanoseconds(
                    TTick time,
                    ClockCallback callback
                ) {
                    if (!callback) {
                        return false;
                    }

                    typename TLockPolicy::Guard
                        lock(_callbacksMutex);

                    for (
                        std::size_t index = 0;
                        index <
                            ESPRESSIO_TIMING_MAX_CALLBACKS;
                        ++index
                    ) {
                        if (
                            !_callbacks[
                                index
                            ].Callback
                        ) {
                            _callbacks[index].Time =
                                time;

                            _callbacks[index].
                                Callback =
                                    std::move(
                                        callback
                                    );

                            return true;
                        }
                    }

                    return false;
                }


                void Update() {
                    const TTick currentTime =
                        GetTimeNanoseconds();

                    ClockCallback
                        callbacks[
                            ESPRESSIO_TIMING_MAX_CALLBACKS
                        ];

                    {
                        typename TLockPolicy::Guard
                            lock(
                                _callbacksMutex
                            );

                        for (
                            std::size_t index = 0;
                            index <
                                ESPRESSIO_TIMING_MAX_CALLBACKS;
                            ++index
                        ) {
                            if (
                                _callbacks[index].
                                    Callback &&
                                currentTime >=
                                    _callbacks[index].
                                        Time
                            ) {
                                callbacks[index] =
                                    std::move(
                                        _callbacks[index].
                                            Callback
                                    );

                                _callbacks[index] =
                                    ScheduledCallback();
                            }
                        }
                    }

                    for (
                        std::size_t index = 0;
                        index <
                            ESPRESSIO_TIMING_MAX_CALLBACKS;
                        ++index
                    ) {
                        if (callbacks[index]) {
                            callbacks[index]();
                        }
                    }
                }


                void ClearCallbacks() {
                    typename TLockPolicy::Guard
                        lock(
                            _callbacksMutex
                        );

                    for (
                        std::size_t index = 0;
                        index <
                            ESPRESSIO_TIMING_MAX_CALLBACKS;
                        ++index
                    ) {
                        _callbacks[index] =
                            ScheduledCallback();
                    }
                }


                ITimeSource*
                GetTimeSource() const {
                    return _timeSource;
                }
        };


        /*
         * Typed facade over the one global SystemClockCore.
         *
         * Different TTime specializations may coexist, but they all represent
         * the same underlying system clock state and callback scheduler.
         */
        template<
            typename TTime = DefaultClockTime,
            typename TLockPolicy =
                ThreadSafeLockPolicy,
            typename TTick = ClockTick
        >
        class SystemClock :
            public ISystemClock<
                TTime
            > {

            private:
                using Core =
                    SystemClockCore<
                        TLockPolicy,
                        TTick
                    >;

                Core& _core;


                SystemClock(
                    ITimeSource* timeSource =
                        HighResolutionTimeSourceT<
                            TLockPolicy
                        >::GetInstance()
                )
                    : _core(
                        Core::GetInstance(
                            timeSource
                        )
                    ) {
                }


            public:
                using TimeType = TTime;
                using TickType = TTick;

                using ClockCallback =
                    typename ISystemClock<
                        TTime
                    >::ClockCallback;


                SystemClock(
                    const SystemClock&
                ) = delete;

                SystemClock& operator=(
                    const SystemClock&
                ) = delete;

                SystemClock(
                    SystemClock&&
                ) = delete;

                SystemClock& operator=(
                    SystemClock&&
                ) = delete;


                static SystemClock&
                GetInstance(
                    ITimeSource* timeSource =
                        HighResolutionTimeSourceT<
                            TLockPolicy
                        >::GetInstance()
                ) {
                    static SystemClock
                        instance(
                            timeSource
                        );

                    return instance;
                }


                TTime GetTime() const override {
                    const TTick resolution =
                        _core.
                            GetResolutionNanoseconds();

                    return
                        TimeTraits<TTime>::
                            template
                            FromNanoseconds<TTick>(
                                _core.
                                    GetTimeNanoseconds(),
                                resolution
                            );
                }


                TTime GetResolution() const override {
                    const TTick resolution =
                        _core.
                            GetResolutionNanoseconds();

                    return
                        TimeTraits<TTime>::
                            template
                            FromNanoseconds<TTick>(
                                resolution,
                                resolution
                            );
                }


                void SetTime(
                    const TTime& time
                ) override {
                    _core.
                        SetTimeNanoseconds(
                            TimeTraits<TTime>::
                                template
                                ToNanoseconds<TTick>(
                                    time
                                )
                        );
                }


                bool TrySetCallback(
                    const TTime& time,
                    ClockCallback callback
                ) {
                    return
                        _core.
                            TrySetCallbackNanoseconds(
                                TimeTraits<TTime>::
                                    template
                                    ToNanoseconds<TTick>(
                                        time
                                    ),
                                std::move(
                                    callback
                                )
                            );
                }


                void SetCallback(
                    const TTime& time,
                    ClockCallback callback
                ) override {
                    TrySetCallback(
                        time,
                        std::move(
                            callback
                        )
                    );
                }


                void Update() override {
                    _core.Update();
                }


                void ClearCallbacks() override {
                    _core.
                        ClearCallbacks();
                }


                ITimeSource*
                GetTimeSource() const {
                    return
                        _core.
                            GetTimeSource();
                }
        };


        template<
            typename TTime = DefaultClockTime,
            typename TTick = ClockTick
        >
        using SingleThreadedSystemClock =
            SystemClock<
                TTime,
                NoLockPolicy,
                TTick
            >;

    }

}
