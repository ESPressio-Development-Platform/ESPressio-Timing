#pragma once

#include <cstdint>

#include <ESPressio_IObserver.hpp>

#include "ESPressio_ClockTypes.hpp"

namespace ESPressio {
namespace Timing {

template<typename TTime, typename TLockPolicy, typename TTick>
class StopwatchClock;

template<
    typename TTime = DefaultClockTime,
    typename TTick = ClockTick
>
class IStopwatchClockObserver :
    public virtual Observable::IObserver {
public:
    virtual ~IStopwatchClockObserver() = default;

    virtual void OnStopwatchStarted(
        TTick elapsedNanoseconds
    ) {
        (void)elapsedNanoseconds;
    }

    virtual void OnStopwatchStopped(
        TTick elapsedNanoseconds
    ) {
        (void)elapsedNanoseconds;
    }

    virtual void OnStopwatchReset(
        TTick previousElapsedNanoseconds,
        bool remainsRunning
    ) {
        (void)previousElapsedNanoseconds;
        (void)remainsRunning;
    }

    virtual void OnStopwatchRestarted(
        TTick previousElapsedNanoseconds
    ) {
        (void)previousElapsedNanoseconds;
    }

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
