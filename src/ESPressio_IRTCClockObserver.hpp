#pragma once

#include <cstdint>

#include <ESPressio_IObserver.hpp>

#include "ESPressio_ClockTypes.hpp"

namespace ESPressio {
namespace Timing {

template<
    typename TTime = DefaultClockTime,
    typename TTick = ClockTick
>
class IRTCClockObserver :
    public virtual Observable::IObserver {
public:
    virtual ~IRTCClockObserver() = default;

    virtual void OnRTCSynchronizationSucceeded(
        TTick previousTimeNanoseconds,
        TTick synchronizedTimeNanoseconds,
        int64_t differenceNanoseconds
    ) {
        (void)previousTimeNanoseconds;
        (void)synchronizedTimeNanoseconds;
        (void)differenceNanoseconds;
    }

    virtual void OnRTCSynchronizationFailed() {}

    virtual void OnRTCInterruptReceived() {}

    virtual void OnRTCInterruptTimeReceived(
        TTick suppliedTimeNanoseconds
    ) {
        (void)suppliedTimeNanoseconds;
    }

    virtual void OnRTCTimeWriteSucceeded(
        TTick previousTimeNanoseconds,
        TTick newTimeNanoseconds,
        int64_t differenceNanoseconds
    ) {
        (void)previousTimeNanoseconds;
        (void)newTimeNanoseconds;
        (void)differenceNanoseconds;
    }

    virtual void OnRTCTimeWriteFailed(
        TTick attemptedTimeNanoseconds
    ) {
        (void)attemptedTimeNanoseconds;
    }
};

} // namespace Timing
} // namespace ESPressio
