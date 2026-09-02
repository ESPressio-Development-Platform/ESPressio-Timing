# Stopwatch Clocks

`StopwatchClock<TTime, ...>` models elapsed time for an independently owned stopwatch instance.

Typical lifecycle operations include start, stop, reset, restart, time set and elapsed-time reads.

```cpp
StopwatchClock<> stopwatch;
stopwatch.Start();
// work
const auto elapsed = stopwatch.GetTime();
stopwatch.Stop();
```

## Public type

The stopwatch's returned `TTime` is selected at compile time through `TimeTraits`; its internal timing state remains in the raw tick/nanosecond domain.

## High-resolution variants

Clock implementations backed by a dedicated System high-resolution counter can present the same stopwatch semantics without exposing the counter's target-native handle/API.

## Observability

`IStopwatchClockObserver<TTime, TTick>` exposes meaningful lifecycle transitions such as started, stopped, reset, restarted and time-set. Ordinary reads do not generate notifications.

See [Timing Observers](Timing-Observers).