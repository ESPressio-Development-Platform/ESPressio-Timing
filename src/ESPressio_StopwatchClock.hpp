#pragma once

#include <memory>

#include "ESPressio_Clock.hpp"
#include "ESPressio_IStopwatchClock.hpp"
#include "ESPressio_IStopwatchClockObserver.hpp"
#include "ESPressio_LockPolicy.hpp"
#include "ESPressio_ThreadSafeLockPolicy.hpp"
#include "ESPressio_TimingObservable.hpp"
#include "ESPressio_TimingObserverUtilities.hpp"

namespace ESPressio {
namespace Timing {

/// <summary>Observable stopwatch implementation backed by a monotonic <c>ITimeSource</c>.</summary>
/// <typeparam name="TLockPolicy">Synchronization policy protecting stopwatch state.</typeparam>
template<
    typename TTime = DefaultClockTime,
    typename TLockPolicy = ThreadSafeLockPolicy,
    typename TTick = ClockTick
>
class StopwatchClock :
    public ClockBase<TTime, TTick>,
    public IStopwatchClock<TTime> {
protected:
    using Base = ClockBase<TTime, TTick>;

    mutable typename TLockPolicy::Mutex _clockMutex;

private:
    TTick _elapsedTime = 0;
    TTick _startTime = 0;
    bool _isRunning = false;

    std::shared_ptr<TimingObservable> _observable =
        CreateTimingObservable();

    TTick GetElapsedNanosecondsLocked(
        TTick sourceTime
    ) const {
        if (!_isRunning) {
            return _elapsedTime;
        }

        const TTick currentInterval =
            sourceTime >= _startTime
                ? sourceTime - _startTime
                : 0;

        return this->AddSaturated(
            _elapsedTime,
            currentInterval
        );
    }

public:
    using TimeType = TTime;
    using TickType = TTick;

    /// <summary>Creates a stopwatch over the supplied time source and optionally starts it immediately.</summary>
    explicit StopwatchClock(
        bool startImmediately = false,
        ITimeSource* timeSource =
            HighResolutionTimeSourceT<TLockPolicy>::GetInstance()
    ) : Base(timeSource) {
        if (startImmediately) {
            Start();
        }
    }

    /// <inheritdoc/>
    void Start() override {
        const TTick sourceTime = this->GetSourceTime();
        TTick elapsed = 0;
        bool changed = false;

        {
            typename TLockPolicy::Guard lock(_clockMutex);

            if (!_isRunning) {
                elapsed = _elapsedTime;
                _startTime = sourceTime;
                _isRunning = true;
                changed = true;
            }
        }

        if (changed) {
            _observable->Notify<
                IStopwatchClockObserver<TTime, TTick>
            >(
                [&](IStopwatchClockObserver<TTime, TTick>* observer) {
                    observer->OnStopwatchStarted(elapsed);
                }
            );
        }
    }

    /// <inheritdoc/>
    void Stop() override {
        const TTick sourceTime = this->GetSourceTime();
        TTick elapsed = 0;
        bool changed = false;

        {
            typename TLockPolicy::Guard lock(_clockMutex);

            if (_isRunning) {
                _elapsedTime =
                    GetElapsedNanosecondsLocked(sourceTime);
                _isRunning = false;
                elapsed = _elapsedTime;
                changed = true;
            }
        }

        if (changed) {
            _observable->Notify<
                IStopwatchClockObserver<TTime, TTick>
            >(
                [&](IStopwatchClockObserver<TTime, TTick>* observer) {
                    observer->OnStopwatchStopped(elapsed);
                }
            );
        }
    }

    /// <inheritdoc/>
    void Reset() override {
        const TTick sourceTime = this->GetSourceTime();
        TTick previousElapsed = 0;
        bool remainsRunning = false;

        {
            typename TLockPolicy::Guard lock(_clockMutex);

            previousElapsed =
                GetElapsedNanosecondsLocked(sourceTime);

            remainsRunning = _isRunning;
            _elapsedTime = 0;
            _startTime = sourceTime;
        }

        _observable->Notify<
            IStopwatchClockObserver<TTime, TTick>
        >(
            [&](IStopwatchClockObserver<TTime, TTick>* observer) {
                observer->OnStopwatchReset(
                    previousElapsed,
                    remainsRunning
                );
            }
        );
    }

    /// <inheritdoc/>
    void Restart() override {
        const TTick sourceTime = this->GetSourceTime();
        TTick previousElapsed = 0;

        {
            typename TLockPolicy::Guard lock(_clockMutex);

            previousElapsed =
                GetElapsedNanosecondsLocked(sourceTime);

            _elapsedTime = 0;
            _startTime = sourceTime;
            _isRunning = true;
        }

        _observable->Notify<
            IStopwatchClockObserver<TTime, TTick>
        >(
            [&](IStopwatchClockObserver<TTime, TTick>* observer) {
                observer->OnStopwatchRestarted(
                    previousElapsed
                );
            }
        );
    }

    /// <inheritdoc/>
    TTime GetTime() const override {
        const TTick sourceTime = this->GetSourceTime();

        typename TLockPolicy::Guard lock(_clockMutex);

        const TTick resolution =
            static_cast<TTick>(
                Internal::GetSourceResolution(
                    this->_timeSource->GetTicksPerSecond()
                )
            );

        return this->CreateTime(
            GetElapsedNanosecondsLocked(sourceTime),
            resolution
        );
    }

    /// <inheritdoc/>
    TTime GetLapTime() const override {
        return GetTime();
    }

    /// <inheritdoc/>
    bool GetIsRunning() const override {
        typename TLockPolicy::Guard lock(_clockMutex);
        return _isRunning;
    }

    /// <inheritdoc/>
    void SetTime(
        const TTime& time
    ) override {
        const TTick sourceTime = this->GetSourceTime();
        const TTick newElapsed = this->GetNanoseconds(time);
        TTick previousElapsed = 0;
        bool running = false;

        {
            typename TLockPolicy::Guard lock(_clockMutex);

            previousElapsed =
                GetElapsedNanosecondsLocked(sourceTime);

            _elapsedTime = newElapsed;
            running = _isRunning;

            if (_isRunning) {
                _startTime = sourceTime;
            }
        }

        _observable->Notify<
            IStopwatchClockObserver<TTime, TTick>
        >(
            [&](IStopwatchClockObserver<TTime, TTick>* observer) {
                observer->OnStopwatchTimeSet(
                    previousElapsed,
                    newElapsed,
                    Internal::SignedDifference(
                        newElapsed,
                        previousElapsed
                    ),
                    running
                );
            }
        );
    }

    /// <summary>Registers an observer for stopwatch lifecycle notifications.</summary>
    Observable::ObserverHandlePtr RegisterObserver(
        IStopwatchClockObserver<TTime, TTick>* observer
    ) {
        return _observable->RegisterObserver(observer);
    }

    /// <summary>Unregisters a stopwatch lifecycle observer.</summary>
    void UnregisterObserver(
        IStopwatchClockObserver<TTime, TTick>* observer
    ) {
        _observable->UnregisterObserver(observer);
    }
};

/// <summary>Stopwatch specialization using a no-op lock policy for single-threaded consumers.</summary>
template<
    typename TTime = DefaultClockTime,
    typename TTick = ClockTick
>
using SingleThreadedStopwatchClock =
    StopwatchClock<TTime, NoLockPolicy, TTick>;

} // namespace Timing
} // namespace ESPressio
