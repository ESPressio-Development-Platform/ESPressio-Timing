# ESPressio Timing

Timing Components of the Flowduino ESPressio Development Platform.

High-resolution system, stopwatch, and RTC clock abstractions with a generic public time representation.

## Version 2.2.0

Version `2.2.0` adds first-class Observer notifications throughout meaningful Timing state transitions, using ESPressio Observable 3.x. The synchronization and generic `TTime` architecture introduced in 2.0/2.1 remains unchanged.

The library no longer defines one globally fixed `ClockTime` contract for every clock. Instead, clock interfaces and implementations are parameterized by their public `TTime` representation.

This allows an application to use ordinary ESPressio Unit time values, opt-in Serializable Unit time values, or another compatible/custom representation without duplicating the Timing algorithms.



## Observer Notifications

Version `2.2.0` makes **ESPressio Observable >= 3.0.0 < 4.0.0** a required dependency and adds observer interfaces for meaningful Timing operations.

Timing deliberately does **not** notify for ordinary reads such as `GetTime()`, `GetResolution()`, `GetIsRunning()`, or `GetSynchronizationStatus()`. Observer callbacks represent operations and state transitions rather than polling activity.

Observable dispatchers are internally owned by `std::shared_ptr`, matching ESPressio Observable's notification-lifetime contract. Timing clocks themselves do not need to inherit from `ThreadSafeObservable`.

Observer exceptions are contained by Timing and do not alter clock state or abort the Timing operation being observed.

### System Clock observers

Implement:

```cpp
ISystemClockObserver<ClockTick>
```

and register through any typed System Clock facade:

```cpp
class ClockObserver :
    public Timing::ISystemClockObserver<
        Timing::ClockTick
    > {
public:
    void OnSystemClockSynchronized(
        Timing::ClockTick before,
        Timing::ClockTick after,
        int64_t immediateDifference,
        const Timing::ClockSynchronizationResult<
            Timing::ClockTick
        >& result,
        const Timing::ClockSynchronizationStatus<
            Timing::ClockTick
        >& status
    ) override {
        // ...
    }
};

ClockObserver observer;

auto handle =
    Timing::SystemClock<>::GetInstance().
        RegisterObserver(&observer);
```

Because the observable belongs to the shared `SystemClockCore`, registering through one `SystemClock<TTime>` facade observes operations performed through every facade sharing that core.

System Clock notifications include:

```text
OnSystemClockTimeSet
OnSystemClockSynchronizationSampleAccepted
OnSystemClockSynchronizationSampleRejected
OnSystemClockSynchronized
OnSystemClockSynchronizationStateChanged
OnSystemClockSynchronizationReset
OnSystemClockSynchronizationConfigurationChanged
OnSystemClockCallbackScheduled
OnSystemClockCallbackScheduleFailed
OnSystemClockCallbackExecuted
OnSystemClockCallbackExecutionFailed
OnSystemClockCallbacksCleared
```

#### Synchronization before/after values

`OnSystemClockSynchronized()` receives:

```text
clockBeforeNanoseconds
clockAfterNanoseconds
immediateDifferenceNanoseconds
ClockSynchronizationResult
ClockSynchronizationStatus
```

The `immediateDifferenceNanoseconds` value describes the **actual immediate public System Clock change caused by processing that synchronization sample**.

This distinction matters for `SlewOnly`:

```text
measured offset       = +2,000,000 ns
pending correction    = +2,000,000 ns
clock before          = 10,000,000 ns
clock after           = 10,000,000 ns
immediate difference  = 0 ns
```

The correction is scheduled for gradual slewing, so reporting a +2 ms instantaneous clock jump would be incorrect.

For `StepIfUnsynchronized` or `StepAlways`, where an immediate step is actually applied, the before/after difference reports that real step.

### Stopwatch observers

`IStopwatchClockObserver<TTime, TTick>` provides callbacks for:

```text
OnStopwatchStarted
OnStopwatchStopped
OnStopwatchReset
OnStopwatchRestarted
OnStopwatchTimeSet
```

Callbacks include relevant elapsed-time values, running state, and before/after difference where applicable.

`GPTimerClock` delegates its observer registration to its internal Stopwatch implementation, so the same Stopwatch observer interface is used.

### RTC observers

`IRTCClockObserver<TTime, TTick>` provides callbacks for:

```text
OnRTCSynchronizationSucceeded
OnRTCSynchronizationFailed
OnRTCInterruptReceived
OnRTCInterruptTimeReceived
OnRTCTimeWriteSucceeded
OnRTCTimeWriteFailed
```

Successful synchronization and write callbacks include the previous and resulting RTC-clock values plus their signed nanosecond difference.

### Observer handles

Registration returns the normal ESPressio Observable owning handle:

```cpp
Observable::ObserverHandlePtr
```

Destroying or explicitly unregistering the handle removes the Observer registration according to the standard ESPressio Observable lifecycle model.

## System Clock Synchronization

Version `2.1.0` adds a transport-independent synchronization and clock-discipline layer to the shared `SystemClockCore`.

The design deliberately separates:

```text
transport
    |
    | captures / carries four timestamps
    v
IClockSynchronizationTarget
    |
    v
ClockDiscipline
    |
    +-- offset estimation
    +-- round-trip-delay estimation
    +-- sample validation/rejection
    +-- phase filtering
    +-- monotonic phase slewing
    +-- residual drift estimation
    +-- continuous rate correction
    |
    v
SystemClockCore
    |
    +-------------------------+
    |                         |
    v                         v
SystemClock<Time>   SystemClock<SerializableTime>
```

Timing has no ESP-NOW, Wi-Fi, UDP, Ethernet, CAN, LoRa, MAC-address, peer, master-election, or packet-format concepts.

A transport library only needs to implement the message exchange required to capture four timestamps:

```text
Local                         Remote

T1 request transmit  -------->
                      <-------- T2 request receive
                      <-------- T3 response transmit
T4 response receive
```

and submit:

```cpp
ClockSynchronizationSample<ClockTick> sample;

sample.LocalRequestTransmitTime   = t1;
sample.RemoteRequestReceiveTime   = t2;
sample.RemoteResponseTransmitTime = t3;
sample.LocalResponseReceiveTime   = t4;

auto result =
    SystemClock<>::GetInstance().
        SubmitSynchronizationSample(
            sample
        );
```

The standard two-way estimates are calculated inside Timing:

```text
round-trip delay =
    (T4 - T1) - (T3 - T2)

clock offset =
    ((T2 - T1) + (T3 - T4)) / 2
```

### Synchronization target interface

Transport implementations should depend on:

```cpp
IClockSynchronizationTarget<ClockTick>
```

rather than on a particular `SystemClock<TTime>` specialization.

The interface exposes:

```cpp
GetSynchronizationTimestampNanoseconds()
SubmitSynchronizationSample(...)
GetSynchronizationStatus()
ConfigureSynchronization(...)
GetSynchronizationConfig()
ResetSynchronization()
```

Because synchronization operates in the raw nanosecond clock domain, it is independent of the public Unit representation.

### Synchronization state

Synchronization status reports:

```cpp
ClockSynchronizationState::Unsynchronized
ClockSynchronizationState::Acquiring
ClockSynchronizationState::Synchronized
```

along with:

```text
last measured offset
filtered offset
pending phase correction
applied correction
round-trip delay
estimated drift in ppm
accepted/rejected sample counts
last accepted sample time
```

A synchronized state can become stale when no accepted sample has arrived within the configured maximum age.

### Sample rejection

Malformed exchanges are rejected.

A configurable maximum round-trip delay also allows a transport to discard high-jitter/high-latency samples before they influence the clock.

### Monotonic phase slewing

The default adjustment mode is:

```cpp
ClockSynchronizationAdjustmentMode::SlewOnly
```

A synchronization sample therefore does not abruptly rebase the System Clock.

Instead, the measured phase error is removed gradually at the configured maximum slew rate. This preserves monotonic progression and is the recommended mode while deadline-driven consumers such as Precision Threads are running.

The default maximum phase slew rate is:

```text
500 ppm
```

and can be configured.

### Startup stepping

Large initial offsets can take a long time to remove with a conservative slew rate.

For startup/bootstrap, before monotonic deadline consumers begin operating, an application may explicitly request:

```cpp
ClockSynchronizationAdjustmentMode::
    StepIfUnsynchronized
```

This permits the first accepted synchronization sample to perform an immediate phase correction. Later samples return to normal slewing.

An explicit:

```cpp
ClockSynchronizationAdjustmentMode::
    StepAlways
```

is also available, but it may move the System Clock forwards or backwards and should not normally be used while monotonic consumers are active.

### Drift estimation and rate correction

Once phase error is settled, successive accepted samples can estimate residual relative clock-rate error.

The estimate is expressed in parts per million:

```text
EstimatedDriftPpm
```

and is applied continuously as a rate correction between synchronization exchanges.

Drift learning is deliberately suspended while a significant phase slew is in progress so intentional phase correction is not mistaken for oscillator drift.

The maximum learned drift correction, learning interval, filtering weights, and phase-learning threshold are configurable.

### Configuration

`ClockSynchronizationConfig` controls:

```text
MaximumRoundTripDelayNanoseconds
MaximumSlewRatePpm
MaximumDriftCorrectionPpm
OffsetFilterWeight
DriftFilterWeight
DriftLearningPhaseThresholdNanoseconds
MinimumDriftLearningIntervalNanoseconds
SynchronizationToleranceNanoseconds
MinimumSamplesForSynchronizedState
MaximumSampleAgeNanoseconds
```

For example:

```cpp
ClockSynchronizationConfig config;

config.MaximumRoundTripDelayNanoseconds =
    10000000ULL; // 10 ms

config.MaximumSlewRatePpm =
    500;

config.SynchronizationToleranceNanoseconds =
    500000ULL; // 0.5 ms

SystemClock<>::GetInstance().
    ConfigureSynchronization(
        config
    );
```

### Shared System Clock semantics

Synchronization is owned by `SystemClockCore`, not by the typed facade.

Therefore:

```cpp
SystemClock<DefaultClockTime>
SystemClock<MyCustomTime>
SystemClock<SerializableClockTime>
```

all observe exactly the same disciplined System Clock on the device.

### SetTime and synchronization

Calling:

```cpp
SystemClock<TTime>::SetTime(...)
```

is an explicit hard rebase.

It resets the current synchronization/discipline state because previously measured phase and drift estimates are no longer valid after the rebase.

### Transport integration

A future ESP-NOW implementation can capture timestamps through the synchronization-target interface and submit complete samples without requiring any ESP-NOW-specific functionality inside ESPressio Timing.

The same Timing API can equally be used by UDP, Ethernet, CAN, serial, LoRa, or another transport.

See:

```text
examples/ClockSynchronization
```


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
    flowduino/ESPressio-Timing@^2.2.1
```

A project selecting Serializable Units additionally declares the appropriate ESPressio Units version and ESPressio Serializable dependency.
