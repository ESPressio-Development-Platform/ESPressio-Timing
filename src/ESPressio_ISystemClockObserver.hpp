#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>

#include <ESPressio_IObserver.hpp>

#include "ESPressio_ClockSynchronization.hpp"
#include "ESPressio_ClockTypes.hpp"

namespace ESPressio {
namespace Timing {

template<typename TLockPolicy, typename TTick>
class SystemClockCore;

/*
 * Observes meaningful state-changing System Clock operations.
 *
 * All time values are expressed in the core's canonical nanosecond domain so
 * observers remain independent of whichever SystemClock<TTime> facade was used
 * to perform the operation.
 */
template<typename TTick = ClockTick>
class ISystemClockObserver :
    public virtual Observable::IObserver {
public:
    virtual ~ISystemClockObserver() = default;

    virtual void OnSystemClockTimeSet(
        TTick previousTimeNanoseconds,
        TTick newTimeNanoseconds,
        int64_t differenceNanoseconds
    ) {
        (void)previousTimeNanoseconds;
        (void)newTimeNanoseconds;
        (void)differenceNanoseconds;
    }

    virtual void OnSystemClockSynchronizationSampleAccepted(
        TTick clockBeforeNanoseconds,
        TTick clockAfterNanoseconds,
        int64_t immediateDifferenceNanoseconds,
        const ClockSynchronizationResult<TTick>& result,
        const ClockSynchronizationStatus<TTick>& status
    ) {
        (void)clockBeforeNanoseconds;
        (void)clockAfterNanoseconds;
        (void)immediateDifferenceNanoseconds;
        (void)result;
        (void)status;
    }

    virtual void OnSystemClockSynchronized(
        TTick clockBeforeNanoseconds,
        TTick clockAfterNanoseconds,
        int64_t immediateDifferenceNanoseconds,
        const ClockSynchronizationResult<TTick>& result,
        const ClockSynchronizationStatus<TTick>& status
    ) {
        (void)clockBeforeNanoseconds;
        (void)clockAfterNanoseconds;
        (void)immediateDifferenceNanoseconds;
        (void)result;
        (void)status;
    }

    virtual void OnSystemClockSynchronizationSampleRejected(
        const ClockSynchronizationResult<TTick>& result,
        const ClockSynchronizationStatus<TTick>& status
    ) {
        (void)result;
        (void)status;
    }

    virtual void OnSystemClockSynchronizationStateChanged(
        ClockSynchronizationState previousState,
        ClockSynchronizationState newState,
        const ClockSynchronizationStatus<TTick>& status
    ) {
        (void)previousState;
        (void)newState;
        (void)status;
    }

    virtual void OnSystemClockSynchronizationReset(
        const ClockSynchronizationStatus<TTick>& previousStatus,
        const ClockSynchronizationStatus<TTick>& newStatus
    ) {
        (void)previousStatus;
        (void)newStatus;
    }

    virtual void OnSystemClockSynchronizationConfigurationChanged(
        const ClockSynchronizationConfig& previousConfig,
        const ClockSynchronizationConfig& newConfig
    ) {
        (void)previousConfig;
        (void)newConfig;
    }

    virtual void OnSystemClockCallbackScheduled(
        TTick scheduledTimeNanoseconds
    ) {
        (void)scheduledTimeNanoseconds;
    }

    virtual void OnSystemClockCallbackScheduleFailed(
        TTick scheduledTimeNanoseconds
    ) {
        (void)scheduledTimeNanoseconds;
    }

    virtual void OnSystemClockCallbackExecuted(
        TTick scheduledTimeNanoseconds,
        TTick actualTimeNanoseconds,
        int64_t differenceNanoseconds
    ) {
        (void)scheduledTimeNanoseconds;
        (void)actualTimeNanoseconds;
        (void)differenceNanoseconds;
    }

    virtual void OnSystemClockCallbackExecutionFailed(
        TTick scheduledTimeNanoseconds,
        TTick actualTimeNanoseconds,
        int64_t differenceNanoseconds,
        std::exception_ptr cause
    ) {
        (void)scheduledTimeNanoseconds;
        (void)actualTimeNanoseconds;
        (void)differenceNanoseconds;
        (void)cause;
    }

    virtual void OnSystemClockCallbacksCleared(
        std::size_t clearedCallbackCount
    ) {
        (void)clearedCallbackCount;
    }
};

} // namespace Timing
} // namespace ESPressio
