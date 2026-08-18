#pragma once

#include <cstddef>
#include <exception>
#include <functional>
#include <limits>
#include <memory>
#include <utility>

#include "ESPressio_ClockTypes.hpp"
#include "ESPressio_ClockDiscipline.hpp"
#include "ESPressio_ISystemClock.hpp"
#include "ESPressio_IClockSynchronizationTarget.hpp"
#include "ESPressio_ISystemClockObserver.hpp"
#include "ESPressio_TimingObservable.hpp"
#include "ESPressio_TimingObserverUtilities.hpp"
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

                mutable ClockDiscipline<TTick>
                    _discipline;

                std::shared_ptr<TimingObservable>
                    _observable =
                        CreateTimingObservable();

                mutable ClockSynchronizationState
                    _lastNotifiedSynchronizationState =
                        ClockSynchronizationState::Unsynchronized;

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


                static TTick ApplySignedCorrection(
                    TTick value,
                    int64_t correction
                ) {
                    if (correction >= 0) {
                        const uint64_t positive =
                            static_cast<uint64_t>(
                                correction
                            );

                        return
                            positive >
                                static_cast<uint64_t>(
                                    std::numeric_limits<
                                        TTick
                                    >::max() -
                                    value
                                )
                                ? std::numeric_limits<
                                    TTick
                                  >::max()
                                : static_cast<TTick>(
                                    value +
                                    static_cast<TTick>(
                                        positive
                                    )
                                  );
                    }

                    const uint64_t magnitude =
                        correction ==
                            std::numeric_limits<
                                int64_t
                            >::min()
                            ? static_cast<uint64_t>(
                                std::numeric_limits<
                                    int64_t
                                >::max()
                              ) +
                              1ULL
                            : static_cast<uint64_t>(
                                -correction
                              );

                    return
                        magnitude >
                            static_cast<uint64_t>(
                                value
                            )
                            ? 0
                            : static_cast<TTick>(
                                value -
                                static_cast<TTick>(
                                    magnitude
                                )
                              );
                }


                TTick GetRawTimeNanosecondsLocked(
                    TTick sourceTime
                ) const {
                    const TTick elapsed =
                        sourceTime >=
                            _baseSourceTime
                            ? sourceTime -
                                _baseSourceTime
                            : 0;

                    return
                        AddSaturated(
                            _baseTime,
                            elapsed
                        );
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

                    TTick correctedTime = 0;
                    ClockSynchronizationStatus<TTick> status;
                    ClockSynchronizationState previousNotifiedState =
                        ClockSynchronizationState::Unsynchronized;
                    bool stateChanged = false;

                    {
                        typename TLockPolicy::Guard
                            lock(_clockMutex);

                        const TTick rawTime =
                            GetRawTimeNanosecondsLocked(
                                sourceTime
                            );

                        _discipline.Advance(
                            rawTime
                        );

                        correctedTime =
                            ApplySignedCorrection(
                                rawTime,
                                _discipline.
                                    GetAppliedCorrectionNanoseconds()
                            );

                        status =
                            _discipline.GetStatus(
                                correctedTime
                            );

                        previousNotifiedState =
                            _lastNotifiedSynchronizationState;

                        if (
                            status.State !=
                            _lastNotifiedSynchronizationState
                        ) {
                            _lastNotifiedSynchronizationState =
                                status.State;
                            stateChanged = true;
                        }
                    }

                    /*
                     * A getter remains silent unless advancing the discipline
                     * causes a genuine synchronization-state transition. That
                     * transition is itself a meaningful clock event.
                     */
                    if (stateChanged) {
                        _observable->Notify<
                            ISystemClockObserver<TTick>
                        >(
                            [&](ISystemClockObserver<TTick>* observer) {
                                observer->OnSystemClockSynchronizationStateChanged(
                                    previousNotifiedState,
                                    status.State,
                                    status
                                );
                            }
                        );
                    }

                    return correctedTime;
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

                    TTick previousTime = 0;
                    ClockSynchronizationStatus<TTick>
                        previousStatus;
                    ClockSynchronizationStatus<TTick>
                        newStatus;

                    {
                        typename TLockPolicy::Guard
                            lock(_clockMutex);

                        const TTick rawTime =
                            GetRawTimeNanosecondsLocked(
                                sourceTime
                            );

                        _discipline.Advance(
                            rawTime
                        );

                        previousTime =
                            ApplySignedCorrection(
                                rawTime,
                                _discipline.
                                    GetAppliedCorrectionNanoseconds()
                            );

                        previousStatus =
                            _discipline.GetStatus(
                                previousTime
                            );

                        _baseTime = time;
                        _baseSourceTime = sourceTime;

                        /*
                         * Explicit SetTime is a hard rebase. The previous
                         * synchronization relationship is invalid afterwards.
                         */
                        _discipline.Reset();

                        newStatus =
                            _discipline.GetStatus(
                                time
                            );

                        _lastNotifiedSynchronizationState =
                            newStatus.State;
                    }

                    const int64_t difference =
                        Internal::SignedDifference(
                            time,
                            previousTime
                        );

                    _observable->Notify<
                        ISystemClockObserver<TTick>
                    >(
                        [&](ISystemClockObserver<TTick>* observer) {
                            observer->OnSystemClockTimeSet(
                                previousTime,
                                time,
                                difference
                            );

                            if (
                                previousStatus.HasAcceptedSample ||
                                previousStatus.State !=
                                    ClockSynchronizationState::Unsynchronized
                            ) {
                                observer->OnSystemClockSynchronizationReset(
                                    previousStatus,
                                    newStatus
                                );

                                if (
                                    previousStatus.State !=
                                    newStatus.State
                                ) {
                                    observer->OnSystemClockSynchronizationStateChanged(
                                        previousStatus.State,
                                        newStatus.State,
                                        newStatus
                                    );
                                }
                            }
                        }
                    );
                }


                TTick
                GetSynchronizationTimestampNanoseconds()
                    const {
                    return
                        GetTimeNanoseconds();
                }


                ClockSynchronizationResult<TTick>
                SubmitSynchronizationSample(
                    const ClockSynchronizationSample<TTick>& sample,
                    ClockSynchronizationAdjustmentMode adjustmentMode =
                        ClockSynchronizationAdjustmentMode::SlewOnly
                ) {
                    ClockSynchronizationResult<TTick> result;
                    ClockSynchronizationStatus<TTick> previousStatus;
                    ClockSynchronizationStatus<TTick> status;
                    TTick clockBefore = 0;
                    TTick clockAfter = 0;
                    ClockSynchronizationState previousNotifiedState =
                        ClockSynchronizationState::Unsynchronized;

                    {
                        const TTick sourceTime =
                            GetSourceTime();

                        typename TLockPolicy::Guard
                            lock(_clockMutex);

                        const TTick rawTime =
                            GetRawTimeNanosecondsLocked(
                                sourceTime
                            );

                        _discipline.Advance(rawTime);

                        clockBefore =
                            ApplySignedCorrection(
                                rawTime,
                                _discipline.
                                    GetAppliedCorrectionNanoseconds()
                            );

                        previousStatus =
                            _discipline.GetStatus(
                                clockBefore
                            );

                        const bool hadAcceptedSample =
                            previousStatus.HasAcceptedSample;

                        result =
                            _discipline.SubmitSample(
                                sample
                            );

                        if (result.Accepted) {
                            const bool applyStep =
                                adjustmentMode ==
                                    ClockSynchronizationAdjustmentMode::StepAlways ||
                                (
                                    adjustmentMode ==
                                        ClockSynchronizationAdjustmentMode::StepIfUnsynchronized &&
                                    !hadAcceptedSample
                                );

                            if (applyStep) {
                                _discipline.ApplyStep(
                                    result.FilteredOffsetNanoseconds
                                );
                            }
                        }

                        clockAfter =
                            ApplySignedCorrection(
                                rawTime,
                                _discipline.
                                    GetAppliedCorrectionNanoseconds()
                            );

                        status =
                            _discipline.GetStatus(
                                clockAfter
                            );

                        previousNotifiedState =
                            _lastNotifiedSynchronizationState;

                        _lastNotifiedSynchronizationState =
                            status.State;
                    }

                    if (!result.Accepted) {
                        _observable->Notify<
                            ISystemClockObserver<TTick>
                        >(
                            [&](ISystemClockObserver<TTick>* observer) {
                                observer->OnSystemClockSynchronizationSampleRejected(
                                    result,
                                    status
                                );
                            }
                        );

                        return result;
                    }

                    const int64_t immediateDifference =
                        Internal::SignedDifference(
                            clockAfter,
                            clockBefore
                        );

                    _observable->Notify<
                        ISystemClockObserver<TTick>
                    >(
                        [&](ISystemClockObserver<TTick>* observer) {
                            observer->OnSystemClockSynchronizationSampleAccepted(
                                clockBefore,
                                clockAfter,
                                immediateDifference,
                                result,
                                status
                            );

                            observer->OnSystemClockSynchronized(
                                clockBefore,
                                clockAfter,
                                immediateDifference,
                                result,
                                status
                            );

                            if (
                                previousNotifiedState !=
                                status.State
                            ) {
                                observer->OnSystemClockSynchronizationStateChanged(
                                    previousNotifiedState,
                                    status.State,
                                    status
                                );
                            }
                        }
                    );

                    return result;
                }


                ClockSynchronizationStatus<TTick>
                GetSynchronizationStatus() const {
                    const TTick sourceTime =
                        GetSourceTime();

                    typename TLockPolicy::Guard
                        lock(_clockMutex);

                    const TTick rawTime =
                        GetRawTimeNanosecondsLocked(
                            sourceTime
                        );

                    _discipline.Advance(
                        rawTime
                    );

                    const TTick correctedTime =
                        ApplySignedCorrection(
                            rawTime,
                            _discipline.
                                GetAppliedCorrectionNanoseconds()
                        );

                    return
                        _discipline.
                            GetStatus(
                                correctedTime
                            );
                }


                void ConfigureSynchronization(
                    const ClockSynchronizationConfig& config
                ) {
                    ClockSynchronizationConfig previousConfig;

                    {
                        typename TLockPolicy::Guard
                            lock(_clockMutex);

                        previousConfig =
                            _discipline.GetConfig();

                        _discipline.Configure(
                            config
                        );
                    }

                    _observable->Notify<
                        ISystemClockObserver<TTick>
                    >(
                        [&](ISystemClockObserver<TTick>* observer) {
                            observer->OnSystemClockSynchronizationConfigurationChanged(
                                previousConfig,
                                config
                            );
                        }
                    );
                }


                ClockSynchronizationConfig
                GetSynchronizationConfig() const {
                    typename TLockPolicy::Guard
                        lock(_clockMutex);

                    return
                        _discipline.
                            GetConfig();
                }


                void ResetSynchronization() {
                    ClockSynchronizationStatus<TTick>
                        previousStatus;
                    ClockSynchronizationStatus<TTick>
                        newStatus;

                    {
                        const TTick sourceTime =
                            GetSourceTime();

                        typename TLockPolicy::Guard
                            lock(_clockMutex);

                        const TTick rawTime =
                            GetRawTimeNanosecondsLocked(
                                sourceTime
                            );

                        _discipline.Advance(rawTime);

                        const TTick correctedTime =
                            ApplySignedCorrection(
                                rawTime,
                                _discipline.
                                    GetAppliedCorrectionNanoseconds()
                            );

                        previousStatus =
                            _discipline.GetStatus(
                                correctedTime
                            );

                        _discipline.Reset();

                        newStatus =
                            _discipline.GetStatus(
                                correctedTime
                            );

                        _lastNotifiedSynchronizationState =
                            newStatus.State;
                    }

                    _observable->Notify<
                        ISystemClockObserver<TTick>
                    >(
                        [&](ISystemClockObserver<TTick>* observer) {
                            observer->OnSystemClockSynchronizationReset(
                                previousStatus,
                                newStatus
                            );

                            if (
                                previousStatus.State !=
                                newStatus.State
                            ) {
                                observer->OnSystemClockSynchronizationStateChanged(
                                    previousStatus.State,
                                    newStatus.State,
                                    newStatus
                                );
                            }
                        }
                    );
                }


                bool TrySetCallbackNanoseconds(
                    TTick time,
                    ClockCallback callback
                ) {
                    if (!callback) {
                        _observable->Notify<
                            ISystemClockObserver<TTick>
                        >(
                            [&](ISystemClockObserver<TTick>* observer) {
                                observer->OnSystemClockCallbackScheduleFailed(
                                    time
                                );
                            }
                        );

                        return false;
                    }

                    bool scheduled = false;

                    {
                        typename TLockPolicy::Guard
                            lock(_callbacksMutex);

                        for (
                            std::size_t index = 0;
                            index < ESPRESSIO_TIMING_MAX_CALLBACKS;
                            ++index
                        ) {
                            if (!_callbacks[index].Callback) {
                                _callbacks[index].Time = time;
                                _callbacks[index].Callback =
                                    std::move(callback);
                                scheduled = true;
                                break;
                            }
                        }
                    }

                    _observable->Notify<
                        ISystemClockObserver<TTick>
                    >(
                        [&](ISystemClockObserver<TTick>* observer) {
                            if (scheduled) {
                                observer->OnSystemClockCallbackScheduled(time);
                            } else {
                                observer->OnSystemClockCallbackScheduleFailed(time);
                            }
                        }
                    );

                    return scheduled;
                }


                void Update() {
                    const TTick currentTime =
                        GetTimeNanoseconds();

                    ClockCallback callbacks[
                        ESPRESSIO_TIMING_MAX_CALLBACKS
                    ];

                    TTick scheduledTimes[
                        ESPRESSIO_TIMING_MAX_CALLBACKS
                    ] = {};

                    {
                        typename TLockPolicy::Guard
                            lock(_callbacksMutex);

                        for (
                            std::size_t index = 0;
                            index < ESPRESSIO_TIMING_MAX_CALLBACKS;
                            ++index
                        ) {
                            if (
                                _callbacks[index].Callback &&
                                currentTime >= _callbacks[index].Time
                            ) {
                                scheduledTimes[index] =
                                    _callbacks[index].Time;

                                callbacks[index] =
                                    std::move(
                                        _callbacks[index].Callback
                                    );

                                _callbacks[index] =
                                    ScheduledCallback();
                            }
                        }
                    }

                    for (
                        std::size_t index = 0;
                        index < ESPRESSIO_TIMING_MAX_CALLBACKS;
                        ++index
                    ) {
                        if (callbacks[index]) {
                            try {
                                callbacks[index]();
                            } catch (...) {
                                const TTick actualTime =
                                    GetTimeNanoseconds();

                                const int64_t difference =
                                    Internal::SignedDifference(
                                        actualTime,
                                        scheduledTimes[index]
                                    );

                                const std::exception_ptr cause =
                                    std::current_exception();

                                _observable->Notify<
                                    ISystemClockObserver<TTick>
                                >(
                                    [&](ISystemClockObserver<TTick>* observer) {
                                        observer->OnSystemClockCallbackExecutionFailed(
                                            scheduledTimes[index],
                                            actualTime,
                                            difference,
                                            cause
                                        );
                                    }
                                );

                                std::rethrow_exception(cause);
                            }

                            const TTick actualTime =
                                GetTimeNanoseconds();

                            const int64_t difference =
                                Internal::SignedDifference(
                                    actualTime,
                                    scheduledTimes[index]
                                );

                            _observable->Notify<
                                ISystemClockObserver<TTick>
                            >(
                                [&](ISystemClockObserver<TTick>* observer) {
                                    observer->OnSystemClockCallbackExecuted(
                                        scheduledTimes[index],
                                        actualTime,
                                        difference
                                    );
                                }
                            );
                        }
                    }
                }


                void ClearCallbacks() {
                    std::size_t clearedCount = 0;

                    {
                        typename TLockPolicy::Guard
                            lock(_callbacksMutex);

                        for (
                            std::size_t index = 0;
                            index < ESPRESSIO_TIMING_MAX_CALLBACKS;
                            ++index
                        ) {
                            if (_callbacks[index].Callback) {
                                ++clearedCount;
                            }

                            _callbacks[index] =
                                ScheduledCallback();
                        }
                    }

                    if (clearedCount > 0) {
                        _observable->Notify<
                            ISystemClockObserver<TTick>
                        >(
                            [&](ISystemClockObserver<TTick>* observer) {
                                observer->OnSystemClockCallbacksCleared(
                                    clearedCount
                                );
                            }
                        );
                    }
                }


                Observable::ObserverHandlePtr
                RegisterObserver(
                    ISystemClockObserver<TTick>* observer
                ) {
                    return _observable->RegisterObserver(observer);
                }


                void UnregisterObserver(
                    ISystemClockObserver<TTick>* observer
                ) {
                    _observable->UnregisterObserver(observer);
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
            >,
            public IClockSynchronizationTarget<
                TTick
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


                TTick
                GetSynchronizationTimestampNanoseconds()
                    const override {
                    return
                        _core.
                            GetSynchronizationTimestampNanoseconds();
                }


                ClockSynchronizationResult<TTick>
                SubmitSynchronizationSample(
                    const ClockSynchronizationSample<TTick>&
                        sample,
                    ClockSynchronizationAdjustmentMode
                        adjustmentMode =
                            ClockSynchronizationAdjustmentMode::
                                SlewOnly
                ) override {
                    return
                        _core.
                            SubmitSynchronizationSample(
                                sample,
                                adjustmentMode
                            );
                }


                ClockSynchronizationStatus<TTick>
                GetSynchronizationStatus()
                    const override {
                    return
                        _core.
                            GetSynchronizationStatus();
                }


                void ConfigureSynchronization(
                    const ClockSynchronizationConfig&
                        config
                ) override {
                    _core.
                        ConfigureSynchronization(
                            config
                        );
                }


                ClockSynchronizationConfig
                GetSynchronizationConfig()
                    const override {
                    return
                        _core.
                            GetSynchronizationConfig();
                }


                void ResetSynchronization() override {
                    _core.
                        ResetSynchronization();
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


                Observable::ObserverHandlePtr
                RegisterObserver(
                    ISystemClockObserver<TTick>* observer
                ) {
                    return
                        _core.RegisterObserver(
                            observer
                        );
                }


                void UnregisterObserver(
                    ISystemClockObserver<TTick>* observer
                ) {
                    _core.UnregisterObserver(
                        observer
                    );
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
