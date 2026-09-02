#pragma once

#include <cstdint>

#include <ESPressio_IObserver.hpp>

#include "ESPressio_ClockTypes.hpp"

namespace ESPressio {
namespace Timing {

template<typename TTime, typename TLockPolicy, typename TTick>
class StopwatchClock;

/// <summary>Observer contract for stopwatch lifecycle and elapsed-time adjustment notifications.</summary>
template<
    typename TTime = DefaultClockTime,
    typename TTick = ClockTick
>
class IStopwatchClockObserver :
    public virtual Observable::IObserver {
public:
    virtual ~IStopwatchClockObserver() = default;

    /// <summary>Called when stopwatch timing begins or resumes.</summary>
    virtual void OnStopwatchStarted(
        TTick elapsedNanoseconds
    ) {
        (void)elapsedNanoseconds;
    }

    /// <summary>Called when stopwatch timing stops.</summary>
    virtual void OnStopwatchStopped(
        TTick elapsedNanoseconds
    ) {
        (void)elapsedNanoseconds;
    }

    /// <summary>Called when the elapsed stopwatch value is reset.</summary>
    virtual void OnStopwatchReset(
        TTick previousElapsedNanoseconds,
        bool remainsRunning
    ) {
        (void)previousElapsedNanoseconds;
        (void)remainsRunning;
    }

    /// <summary>Called when the stopwatch is reset and immediately restarted.</summary>
    virtual void OnStopwatchRestarted(
        TTick previousElapsedNanoseconds
    ) {
        (void)previousElapsedNanoseconds;
    }

    /// <summary>Called after the elapsed stopwatch value is explicitly changed.</summary>
    virtual void OnStopwatchTimeSet(
        TTick previousElapsedNanoseconds,
        TTick newElapsedNanoseconds,
        int64_t differenceNanoseconds,
        bool isRunning
    ) {
        (void)previousElapsedNanoseconds;
        (void)newElapsedNanoseconds;
        (void)differenceNanoseconds;
        (void)isRunning;
    }
};

} // namespace Timing
} // namespace ESPressio
