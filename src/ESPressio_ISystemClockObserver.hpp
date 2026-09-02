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

/// <summary>Observes meaningful state changes performed by the System Clock.</summary>
/// <remarks>All timestamps are expressed in the core canonical nanosecond domain, independent of the public <c>SystemClock&lt;TTime&gt;</c> facade.</remarks>
template<typename TTick = ClockTick>
class ISystemClockObserver :
    public virtual Observable::IObserver {
public:
    virtual ~ISystemClockObserver() = default;

    /// <summary>Called after the System Clock time is explicitly set.</summary>
    virtual void OnSystemClockTimeSet(
        TTick previousTimeNanoseconds,
        TTick newTimeNanoseconds,
        int64_t differenceNanoseconds
    ) {
        (void)previousTimeNanoseconds;
        (void)newTimeNanoseconds;
        (void)differenceNanoseconds;
    }

    /// <summary>Called after a synchronization sample is accepted by the discipline engine.</summary>
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

    /// <summary>Called when an accepted sample causes the clock to enter synchronized state.</summary>
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

    /// <summary>Called when a submitted synchronization sample is rejected.</summary>
    virtual void OnSystemClockSynchronizationSampleRejected(
        const ClockSynchronizationResult<TTick>& result,
        const ClockSynchronizationStatus<TTick>& status
    ) {
        (void)result;
        (void)status;
    }

    /// <summary>Called whenever the synchronization acquisition state changes.</summary>
    virtual void OnSystemClockSynchronizationStateChanged(
        ClockSynchronizationState previousState,
        ClockSynchronizationState newState,
        const ClockSynchronizationStatus<TTick>& status
    ) {
        (void)previousState;
        (void)newState;
        (void)status;
    }

    /// <summary>Called after synchronization history and discipline state are reset.</summary>
    virtual void OnSystemClockSynchronizationReset(
        const ClockSynchronizationStatus<TTick>& previousStatus,
        const ClockSynchronizationStatus<TTick>& newStatus
    ) {
        (void)previousStatus;
        (void)newStatus;
    }

    /// <summary>Called after synchronization configuration is replaced.</summary>
    virtual void OnSystemClockSynchronizationConfigurationChanged(
        const ClockSynchronizationConfig& previousConfig,
        const ClockSynchronizationConfig& newConfig
    ) {
        (void)previousConfig;
        (void)newConfig;
    }

    /// <summary>Called after a clock callback is successfully scheduled.</summary>
    virtual void OnSystemClockCallbackScheduled(
        TTick scheduledTimeNanoseconds
    ) {
        (void)scheduledTimeNanoseconds;
    }

    /// <summary>Called when a clock callback cannot be scheduled.</summary>
    virtual void OnSystemClockCallbackScheduleFailed(
        TTick scheduledTimeNanoseconds
    ) {
        (void)scheduledTimeNanoseconds;
    }

    /// <summary>Called after a scheduled clock callback executes successfully.</summary>
    virtual void OnSystemClockCallbackExecuted(
        TTick scheduledTimeNanoseconds,
        TTick actualTimeNanoseconds,
        int64_t differenceNanoseconds
    ) {
        (void)scheduledTimeNanoseconds;
        (void)actualTimeNanoseconds;
        (void)differenceNanoseconds;
    }

    /// <summary>Called when a scheduled clock callback throws during execution.</summary>
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

    /// <summary>Called after pending scheduled callbacks are cleared.</summary>
    virtual void OnSystemClockCallbacksCleared(
        std::size_t clearedCallbackCount
    ) {
        (void)clearedCallbackCount;
    }
};

} // namespace Timing
} // namespace ESPressio
