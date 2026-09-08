#pragma once

#include <cstdint>

#include <ESPressio_IObserver.hpp>

#include "ESPressio_ClockTypes.hpp"

namespace ESPressio {
namespace Timing {

/// <summary>Observer contract for RTC synchronization, interrupt, and explicit RTC-write lifecycle notifications.</summary>
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members: none; polymorphic interface/object includes vptr storage where not supplied by a base.
 * Total Memory: 4 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
template<
    typename TTime = DefaultClockTime,
    typename TTick = ClockTick
>
class IRTCClockObserver :
    public virtual Observable::IObserver {
public:
    virtual ~IRTCClockObserver() = default;

    /// <summary>Called after the clock successfully synchronizes from its RTC source.</summary>
    virtual void OnRTCSynchronizationSucceeded(
        TTick previousTimeNanoseconds,
        TTick synchronizedTimeNanoseconds,
        int64_t differenceNanoseconds
    ) {
        (void)previousTimeNanoseconds;
        (void)synchronizedTimeNanoseconds;
        (void)differenceNanoseconds;
    }

    /// <summary>Called when synchronization from the RTC source fails.</summary>
    virtual void OnRTCSynchronizationFailed() {}

    /// <summary>Called when an RTC interrupt is received without an externally supplied timestamp.</summary>
    virtual void OnRTCInterruptReceived() {}

    /// <summary>Called when an RTC interrupt includes a timestamp captured by the caller.</summary>
    virtual void OnRTCInterruptTimeReceived(
        TTick suppliedTimeNanoseconds
    ) {
        (void)suppliedTimeNanoseconds;
    }

    /// <summary>Called after an explicit RTC time write succeeds.</summary>
    virtual void OnRTCTimeWriteSucceeded(
        TTick previousTimeNanoseconds,
        TTick newTimeNanoseconds,
        int64_t differenceNanoseconds
    ) {
        (void)previousTimeNanoseconds;
        (void)newTimeNanoseconds;
        (void)differenceNanoseconds;
    }

    /// <summary>Called when an explicit RTC time write fails.</summary>
    virtual void OnRTCTimeWriteFailed(
        TTick attemptedTimeNanoseconds
    ) {
        (void)attemptedTimeNanoseconds;
    }
};

} // namespace Timing
} // namespace ESPressio
