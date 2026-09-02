# Timing Observers

Timing uses ESPressio Observable for meaningful operations and state transitions, not for ordinary polling reads.

## System Clock

`ISystemClockObserver<ClockTick>` can observe time set, synchronization sample acceptance/rejection, synchronization completion/state/reset/configuration, and scheduled callback lifecycle.

Because observers attach to the shared `SystemClockCore`, registration through one typed facade observes operations performed through every facade sharing that core.

## Stopwatch

`IStopwatchClockObserver<TTime, TTick>` reports start, stop, reset, restart and explicit time-set operations.

## RTC

`IRTCClockObserver<TTime, TTick>` reports synchronization success/failure, interrupt receipt/time receipt and time-write success/failure.

## Callback failures

Observer exceptions are contained by Timing so an observer does not roll back clock state or abort the timing operation being observed.

## Lifetime

Registration returns the normal ESPressio Observable owning handle. Destroying or explicitly unregistering it ends observation.