# Clock Model

Timing separates raw measurement from public representation:

```text
platform clock/counter
        |
        v
ClockTick / TTick
        |
        v
clock algorithms and state
        |
        v
TimeTraits<TTime>
        |
        v
public TTime
```

Raw clock arithmetic remains numeric and nanosecond-oriented. `TimeTraits<TTime>` converts between that domain and the selected public type.

## Clock families

Timing exposes interfaces and implementations for settable/system clocks, stopwatches, RTC-oriented clocks and high-resolution stopwatch-style clocks.

## System versus instance clocks

`SystemClock<TTime>` is a typed view over one global `SystemClockCore`. User-created stopwatches and similar clocks own independent state because separate instances are semantically meaningful.

## Live clocks are not persistence objects

A live monotonic clock contains runtime/hardware epochs that are not generally meaningful after restart. If an application needs persistent stopwatch state, persist an explicit snapshot DTO with defined restoration semantics rather than serializing the clock implementation itself.