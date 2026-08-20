# Changelog

## 2.2.3 — 2026-08-20

### Changed
- Raised the required ESPressio Units baseline from 0.2.1 to 0.2.2 following the Serializable 0.10.1 dependency refresh.
- Preserved the ESPressio Observable baseline at `>=3.0.1 <4.0.0`.
- Preserved Timing's architecture in which Serializable time representations are supplied through opt-in Serializable Unit types rather than a direct Timing -> Serializable dependency.
- Updated package metadata and current dependency documentation for Timing 2.2.3.

### Compatibility
- No Timing public API or runtime behaviour changes are introduced by this dependency-maintenance release.

## 2.2.2 — 2026-08-20

### Changed
- Raised the minimum ESPressio Observable dependency from 3.0.0 to 3.0.1 so Timing explicitly consumes the latest validated Observable 3.x patch release and its zero-observer notification fast path.
- Preserved the existing Timing 2.2.x public API and behaviour; this release is dependency-maintenance only.

## 2.2.1 — 2026-08-19

### Changed
- Updated active ESPressio dependency baselines to the latest released versions available on 2026-08-19.
- Bounded dependency compatibility to the current major version so future breaking major releases are not selected automatically.
- Updated the required ESPressio Units baseline to 0.2.1 within the 0.x line after the Units dependency-refresh release.

## 2.2.0

Additive Observable integration release.

### Observable dependency

- Added required dependency on ESPressio Observable `>=3.0.0`.
- Added internally shared-owned `TimingObservable` dispatchers compatible with Observable 3.x notification lifetime semantics.
- Observer callback failures are contained and cannot change Timing operation results.

### System Clock observers

- Added `ISystemClockObserver<TTick>`.
- Added notifications for explicit `SetTime()` rebases, including previous/new values and signed difference.
- Added accepted and rejected synchronization-sample notifications.
- Added `OnSystemClockSynchronized()` with actual clock value before synchronization processing, immediate clock value afterwards, signed immediate difference, full synchronization result, and resulting status.
- Added synchronization-state transition notifications.
- Added synchronization reset and configuration-change notifications.
- Added callback scheduled/scheduling-failed/executed/execution-failed notifications.
- Added callback-clear notification including number of removed callbacks.
- System Clock observers are owned by the shared `SystemClockCore`, so all typed `SystemClock<TTime>` facades expose the same observer channel.

### Stopwatch observers

- Added `IStopwatchClockObserver<TTime, TTick>`.
- Added notifications for Start, Stop, Reset, Restart, and SetTime.
- Added relevant elapsed before/after values, signed differences, and running-state information.
- `GPTimerClock` forwards Stopwatch observer registration to its internal Stopwatch.

### RTC observers

- Added `IRTCClockObserver<TTime, TTick>`.
- Added synchronization success/failure notifications.
- Added RTC interrupt notifications, including supplied interrupt time when present.
- Added RTC write success/failure notifications.
- Successful synchronization/write notifications include previous/new values and signed difference.

### Tests

- Added observer regression coverage for System Clock synchronization, rejected samples, state changes, scheduled callbacks, callback failures, Stopwatch operations, and RTC operations.

## 2.1.0

Additive synchronization release.

### System Clock synchronization

- Added transport-independent `ClockSynchronizationSample<TTick>`.
- Added NTP-style four-timestamp offset and round-trip-delay estimation.
- Added malformed/high-delay sample rejection.
- Added `ClockSynchronizationConfig`, result and status types.
- Added `ClockSynchronizationState`.
- Added `ClockSynchronizationAdjustmentMode`.
- Added `IClockSynchronizationTarget<TTick>` for transport integrations.
- Added `ClockDiscipline<TTick>`.
- Added monotonic phase slewing with configurable maximum slew rate.
- Added explicit startup stepping through `StepIfUnsynchronized`.
- Added explicit `StepAlways` mode for callers that knowingly permit discontinuous rebasing.
- Added filtered residual drift estimation and continuous ppm rate correction.
- Added synchronization staleness handling and accepted/rejected sample counters.
- Integrated discipline into the single shared `SystemClockCore`, so all `SystemClock<TTime>` facades observe the same synchronized timeline.
- `SystemClock<TTime>::SetTime()` now resets synchronization state after an explicit hard rebase.
- Added transport-neutral synchronization example and host regression tests.

## 2.0.0

Major architectural release.

### Breaking changes

- `IClock`, `IClockSettable`, `IStopwatchClock`, `ISystemClock`, and `IRTCClock` are now templates over `TTime`.
- `StopwatchClock`, `SystemClock`, `RTCClockBase`, and `GPTimerClock` are now primary class templates.
- The global `ClockTime` API contract has been replaced by `DefaultClockTime` plus each clock's `TimeType`.
- `SetTime()` and related APIs accept `const TTime&`.
- Public time representation and raw tick-storage representation are independent template parameters.

### New architecture

- `SystemClock<TTime>` is now a typed facade over one shared `SystemClockCore<TLockPolicy, TTick>` singleton, so different time representations observe the same global timeline.

- Added `TimeTraits<TTime>` customization point.
- Added configurable raw `TTick` storage/arithmetic type.
- Raw time-source APIs remain independent of public Unit representation.
- Ordinary ESPressio Units remain the default.
- Optional Serializable Unit types can be selected as `TTime` without ESPressio Timing depending upon ESPressio Serializable.
