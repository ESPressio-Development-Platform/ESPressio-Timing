# ESPressio Timing

Timing Components of the Flowduino ESPressio Development Platform.

High-resolution system, stopwatch, and RTC clock abstractions with a generic public time representation.

## Version 2.0.0

Version `2.0.0` is a deliberate breaking architectural release.

The library no longer defines one globally fixed `ClockTime` contract for every clock. Instead, clock interfaces and implementations are parameterized by their public `TTime` representation.

This allows an application to use ordinary ESPressio Unit time values, opt-in Serializable Unit time values, or another compatible/custom representation without duplicating the Timing algorithms.

## Core Design

Timing separates three concerns:

```text
Raw time source
      |
      v
ClockTick / TTick
      |
      | timing arithmetic
      v
Clock implementation
      |
      v
TimeTraits<TTime>
      |
      v
Public TTime representation
```

Raw timing state remains numeric and independent of serialization.

The public representation is selected at compile time.

## Default Time Representation

The default is:

```cpp
using DefaultClockTime =
    ESPressio::Units::Time<
        uint64_t,
        ESPressio::Units::Nano
    >;
```

Therefore:

```cpp
StopwatchClock<> stopwatch;
```

uses ordinary, non-Serializable ESPressio Units.

Timing itself does not depend on ESPressio Serializable.

## Selecting Another Time Representation

Every clock and clock interface exposes its time representation as a template parameter:

```cpp
IClock<TTime>
IClockSettable<TTime>
IStopwatchClock<TTime>
ISystemClock<TTime>
IRTCClock<TTime>

StopwatchClock<TTime, TLockPolicy, TTick>
SystemClock<TTime, TLockPolicy, TTick>
RTCClockBase<TTime, TLockPolicy, TTick>
GPTimerClock<TTime, TLockPolicy, TTick>
```

For example:

```cpp
using SerializableClockTime =
    Units::SerializableNanoSeconds<
        uint64_t
    >;

StopwatchClock<
    SerializableClockTime
> stopwatch;

SerializableClockTime elapsed =
    stopwatch.GetTime();
```

Only that consuming project needs to include the Serializable Unit header and depend upon ESPressio Serializable.

The clock itself is not serialized; its returned `TimeType` is serializable.

## Why Clocks Are Not Called "SerializableClock"

A monotonic clock contains runtime/hardware state such as source ticks and synchronization epochs. Persisting that internal state across restart is generally not meaningful.

For example, restoring an old monotonic `_startTime` after reboot would be incorrect.

If persistent stopwatch/clock state is required, it should be represented by an explicit snapshot DTO with defined restoration semantics rather than by serializing the live clock implementation.


## System Clock Singleton Semantics

`SystemClock<TTime>` is a typed facade over one non-templated-by-time
`SystemClockCore<TLockPolicy, TTick>` singleton.

This is important because the system clock represents one global timeline.
Different public time representations must not create independent clocks.

For example:

```cpp
using SerializableClockTime =
    Units::SerializableNanoSeconds<uint64_t>;

auto& ordinary =
    SystemClock<
        DefaultClockTime
    >::GetInstance();

auto& serializable =
    SystemClock<
        SerializableClockTime
    >::GetInstance();
```

These are two typed views over the **same underlying SystemClockCore**.

Therefore:

```cpp
ordinary.SetTime(...);

auto serializedTime =
    serializable.GetTime();
```

observes the same clock state.

Callbacks are also stored by the shared core in raw nanosecond ticks. A callback
registered through one `SystemClock<TTime>` specialization can be serviced by
calling `Update()` through another specialization.

The tiny typed facade objects themselves are template specializations, but they
contain no independent timeline or scheduler state.

This singleton-core/view distinction applies specifically to `SystemClock`.
User-created clocks such as `StopwatchClock<TTime>` continue to own their own raw
timing state because separate stopwatch instances are semantically meaningful.

## TimeTraits

`TimeTraits<TTime>` connects the public representation to Timing's internal nanosecond domain.

The default specialization supports types exposing:

```text
value
orderOfMagnitude
```

and constructible from:

```cpp
TTime(value, magnitude)
```

This includes ordinary ESPressio `Time` types and their optional Serializable wrappers.

Unrelated representations can be integrated by explicitly specializing:

```cpp
template<>
struct ESPressio::Timing::TimeTraits<MyTime> {
    template<typename TTick>
    static MyTime FromNanoseconds(
        TTick nanoseconds,
        TTick resolution
    );

    template<typename TTick>
    static TTick ToNanoseconds(
        const MyTime& time
    );
};
```

## Raw Tick Type

The storage/arithmetic type is separately configurable:

```cpp
StopwatchClock<
    MyTime,
    ThreadSafeLockPolicy,
    uint64_t
>
```

The default is:

```cpp
ClockTick
```

which is currently `uint64_t`.

Serialization properties of `TTime` therefore do not leak into the timing algorithm or raw clock storage.

## Interfaces

Interfaces are now generic:

```cpp
IClock<DefaultClockTime>
IClock<MyApplicationTime>
```

These are intentionally distinct C++ contracts.

Generic consuming code should use the clock's nested type:

```cpp
template<typename TClock>
void ReadClock(
    TClock& clock
) {
    typename TClock::TimeType now =
        clock.GetTime();
}
```

rather than assuming a global `ClockTime`.

## Thread Safety

The lock policy remains a compile-time parameter.

The normal default is:

```cpp
ThreadSafeLockPolicy
```

For single-context applications:

```cpp
SingleThreadedStopwatchClock<MyTime>
SingleThreadedSystemClock<MyTime>
SingleThreadedRTCClockBase<MyTime>
```

use `NoLockPolicy`.

## Dependency Model

Ordinary Timing project:

```text
Application
    |
    +-- ESPressio-Timing
            |
            +-- ESPressio-Units
```

No ESPressio Serializable dependency is required.

Application selecting a Serializable Unit as `TTime`:

```text
Application
    |
    +-- ESPressio-Timing
    |
    +-- ESPressio-Units
    |       |
    |       +-- optional *_Serializable.hpp
    |
    +-- ESPressio-Serializable
```

Timing remains unaware of ESPressio Serializable.

## Migration From 1.x

Version 1.x:

```cpp
StopwatchClock stopwatch(true);
ClockTime elapsed =
    stopwatch.GetTime();

const IClock& clock =
    stopwatch;
```

Version 2.x:

```cpp
StopwatchClock<> stopwatch(true);

DefaultClockTime elapsed =
    stopwatch.GetTime();

const IClock<
    DefaultClockTime
>& clock =
    stopwatch;
```

Generic code should preferably use:

```cpp
typename TClock::TimeType
```

instead of naming `DefaultClockTime`.

## PlatformIO

```ini
lib_deps =
    flowduino/ESPressio-Timing@^2.0.0
```

A project selecting Serializable Units additionally declares the appropriate ESPressio Units version and ESPressio Serializable dependency.
