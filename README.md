# ESPressio Timing

Timing provides unit-aware clocks and a bounded, transport-neutral distributed
clock discipline. It depends on System, Units and Observable on their coordinated
`primitives_redesign` branches. It has no Task, Threads, Radio, Mesh, Primitive or
Serializable dependency. Applications choosing a Serializable time representation
add that optional dependency through Units.

The default synchronized System timeline maps `System::Clock::Monotonic()` to
System nanoseconds. Synchronization never changes that raw duration/deadline clock.
An explicit `ITimeSource` can be supplied when composing a test or a different
validated source. All typed SystemClock facades sharing a lock/tick policy use the
same core, continuity seal, estimator and callback scheduler.

Include `ESPressio_TimingSystemClock.hpp` for an unambiguous synchronized clock,
`ESPressio_MonotonicClock.hpp` for a duration clock, or `ESPressio_Timing.hpp` for the
complete library. Install concrete System providers before constructing/initializing
clock services. Singleton construction and optional observer registration are
bootstrap work; clock/model/status getters allocate nothing and invoke no callbacks.

## Bootstrap and continuous time

```cpp
#include <ESPressio_TimingSystemClock.hpp>
using namespace ESPressio::Timing;
void BootstrapClock() {
    auto& clock = SystemClock<>::GetInstance();
    auto rebased = clock.TrySetTime(DefaultClockTime(0, ESPressio::Units::Nano));
    if (rebased != ClockConfigurationStatus::Success) return;
    clock.SealContinuity();
    auto refused = clock.TrySetTime(DefaultClockTime(1, ESPressio::Units::Base));
    // refused == ContinuitySealed; time and synchronization state are unchanged.
    auto origin = clock.CaptureQualifiedTime();
    auto model = clock.GetClockModelSnapshot();
    (void)refused; (void)origin; (void)model;
}
```

`ISystemClock<TTime>` exposes status-returning `TrySetTime`, `SealContinuity` and
`IsContinuitySealed`. It does not derive from unconditional `IClockSettable`.
Explicit rebases are allowed only before sealing. Synchronization always publishes
a model anchored at the old model's current value and removes phase error by slew.
Frequency correction plus slew has a strictly positive effective rate. The integer
reader rounds the combined active rate once to prevent nanosecond backward jumps.
There is no synchronization step mode or runtime unseal/reset escape hatch.

`CaptureQualifiedTime()` returns one `{Nanoseconds, Reliability}` snapshot suitable
for semantic origin/truth capture before a family-owned bounded wait. Receivers
preserve those fields; they do not recompute or upgrade the origin's reliability.

## Complete capture evidence

```cpp
#include <ESPressio_TimingSystemClock.hpp>
using namespace ESPressio::Timing;
ClockConfigurationStatus ConfigureReference(IClockSynchronizationTarget& target) {
    ClockSynchronizationProfile profile;
    profile.MinimumAcceptedSamples = 4;
    profile.MinimumRegressionObservationSpanNanoseconds = 500000000;
    profile.ResidualFrequencyErrorBoundPpm = 50; // requires validation on the target
    auto configured = target.ConfigureSynchronization(profile);
    if (configured != ClockConfigurationStatus::Success) return configured;
    return target.SelectSynchronizationReference(1); // trusted lineage token from orchestration
}
ClockSynchronizationResult SubmitExchange(
    IClockSynchronizationTarget& target,
    ClockTimestampCapture<> t1, ClockTimestampCapture<> t2,
    ClockTimestampCapture<> t3, ClockTimestampCapture<> t4,
    std::uint64_t reference, TimeReliability referenceQuality,
    ClockUncertainty referenceUncertainty) {
    ClockSynchronizationObservation<> observation;
    observation.T1 = t1; observation.T2 = t2;
    observation.T3 = t3; observation.T4 = t4;
    observation.ReferenceIdentity = reference;
    observation.ReferenceReliability = referenceQuality;
    observation.ReferenceUncertainty = referenceUncertainty;
    return target.SubmitSynchronizationObservation(observation);
}
```

T1 is local request transmit, T2 remote request receive, T3 remote response transmit,
and T4 local response receive. Each `ClockTimestampCapture` preserves System and raw
monotonic coordinates plus capture quality and uncertainty. The observation
coordinate is the local raw-monotonic midpoint. Timing calculates the locked
four-System-timestamp offset and RTT; transports do not perform discipline.

The default capture is deliberately acquisition-only:

```cpp
#include <ESPressio_TimingSystemClock.hpp>
void CaptureWithoutLatencyProof() {
    auto& clock = ESPressio::Timing::SystemClock<>::GetInstance();
    auto capture = clock.CaptureSynchronizationTimestamp();
    // SoftwareUnbounded and unknown uncertainty cannot support qualified time.
    (void)capture;
}
```

A provider with a validated finite capture bound may pass Hardware or
SoftwareBounded and `ClockUncertainty::Known(bound)`. Zero/invalid capture quality
is rejected. Unknown/unbounded error can support acquisition and diagnostics but
cannot certify Synchronized/Holdover. A later worker-service timestamp must not be
presented as a precise earlier RX/TX boundary. Providers retain the model appropriate
to a raw hardware capture when converting it; they must never reconstruct an old
System timestamp from a later mutable System time minus elapsed duration.

The conservative observation bound is reference uncertainty + ceil(RTT/2) +
ceil(sum of the four capture bounds / 2). Only an explicit trusted calibrated path
proof may replace ceil(RTT/2) with a smaller asymmetry bound. Invalid ordering,
impossible processing elapsed time, arithmetic overflow, unknown reference token,
invalid quality, excessive bounded capture error and RTT above the finite ceiling
are rejected. Zero is not an unlimited RTT setting.

## Fixed affine estimation and safety

`ClockRegression<N>` and `ClockDiscipline<N>` require compile-time N >= 4. The System
core uses N=8. The first uncertainty-weighted affine least-squares fit determines
residual inclusion, followed by exactly one second fit. There is no iterative
solver, dynamic window, minimum-delay winner or successive-pair drift learner.
Coordinates are centered near the newest observation. Original measured offset
is retained; capture-consistent reference-minus-monotonic normalization prevents
local servo changes between exchanges from appearing as oscillator drift.

`ClockDiscipline` stages one fixed candidate window before acceptance. A rejected
outlier leaves the retained window and published mapping intact. Reference changes
clear evidence and return to acquisition while preserving current public time.
Invalid profile configuration changes nothing. The independent physical
`ResidualFrequencyErrorBoundPpm` is never reduced by regression confidence or
sample count. Quantized/clipped correction error and numerical/source guards are
added conservatively.

At each accepted anchor the safety budget includes observation uncertainty,
retained inlier residuals, the full unresolved phase correction and quantization.
It grows with elapsed raw monotonic time. Slew settlement alone cannot make the
clock more certain: only accepted new evidence can establish a tighter anchor.

```cpp
#include <ESPressio_ClockDiscipline.hpp>
void InspectBoundedDiscipline() {
    ESPressio::Timing::ClockDiscipline<8> discipline;
    ESPressio::Timing::ClockSynchronizationProfile profile;
    auto configured = discipline.Configure(profile, 0);
    auto reference = discipline.SelectReference(1, 0);
    discipline.SealContinuity();
    auto status = discipline.GetStatus(0);
    auto snapshot = discipline.Model();
    (void)configured; (void)reference; (void)status; (void)snapshot;
}
```

The discipline owns N numeric observation records and one compact model/status.
Submission uses a fixed N-record candidate plus bounded regression arrays; no
allocation or runtime resizing occurs. `ClockModelSnapshot::Evaluate` uses integer
quotient/remainder arithmetic without a target-dependent 128-bit requirement.
Optional Observable diagnostics and general scheduled callback closures are
separate from this deterministic model. Ordinary clock/status/model reads never
invoke those mechanisms.

## Qualification and adaptive deadlines

| Label | Meaning |
|---|---|
| Unqualified (0) | No qualified model and no active acquisition. |
| Acquiring (1) | Active acquisition without sufficient maturity or uncertainty headroom. |
| Synchronized (2) | Mature, qualified reference evidence; current uncertainty strictly below 1 ms. |
| Holdover (3) | Previously synchronized, fresh reference temporarily absent, still strictly below 1 ms. |

These labels are not an ordinal score. Entry from Acquiring requires at least four
inliers, the configured positive observation span, a qualified reference and
uncertainty <=500 us. Reaching exactly 1,000,000 ns immediately removes the
qualified claim. With active reacquisition the result is Acquiring; otherwise it
is Unqualified. Recovery after expiry must pass the <=500 us entry gate again.

```cpp
#include <ESPressio_TimingSystemClock.hpp>
void ServiceReferenceChange() {
    auto& clock = ESPressio::Timing::SystemClock<>::GetInstance();
    clock.SetSynchronizationActivity(true, false); // qualified model enters bounded Holdover
    auto status = clock.GetSynchronizationStatus();
    if (status.HasSynchronizationDeadline) {
        // The transport schedules against this raw monotonic deadline, using
        // its own wake/deadline service and conservative airtime/capture budget.
        auto deadline = status.NextRequiredSynchronizationMonotonic;
        (void)deadline;
    }
    clock.RecordSynchronizationDeadlineMiss(); // call only for an actual scheduler miss
    clock.SelectSynchronizationReference(2);   // new lineage; old samples are discarded
    clock.Update();                           // explicit diagnostics/callback service
}
```

The deadline derives from uncertainty headroom, operational guard and physical
residual drift, capped by the profile's explicit cadence limits and fresh-sample
age. Too little headroom can make the deadline immediately due; minimum cadence
never postpones work past the safe bound. Acquisition uses its configured cadence.
There is no universal 1 Hz refresh, hidden timer task or polling loop. Radio normally
owns the atomic direct-neighbor exchange, Mesh owns reference/topology selection,
and Timing owns the estimator and uncertainty. Clock traffic still uses protected
capacity and the shared fair transport scheduler.

Status reports reference identity, last accepted raw-monotonic coordinate, sample
age, RTT, measured/model residuals, frequency correction, physical drift bound,
current uncertainty, pending phase, inlier/count/span facts, rejection counters,
capture quality counters, scheduler misses and the next required deadline.
Numeric profile defaults and host tests are not hardware sub-ms certification.
Certification requires validated capture/oscillator bounds under the specified
hardware, temperature, power, saturation and multi-hop conditions.

## General clock representations and observers

`IClock<TTime>` returns a unit-aware value. `TimeTraits<TTime>` converts to/from the
internal nanosecond coordinate while preserving source resolution. The default is
`Units::Time<uint64_t, Units::Nano>`; optional Serializable Units representations
use the same algorithms and shared System core. `StopwatchClock`, `RTCClockBase`
and `GPTimerClock` retain their explicit settable/start/stop/RTC semantics.

`MonotonicClock` remains independent of System rebasing/slewing. Shared
`HighResolutionTimeSource` uses the installed System counter provider, with a
System monotonic fallback when unavailable at initialization. The explicit
provider-backed `GPTimerTimeSource`/`GPTimerClock` surfaces report availability and
`PlatformResult`; Timing includes no ESP-IDF timer headers or native status types.
Use `GetIsUsingHighResolutionCounter()` for shared-source diagnostics.

System, Stopwatch and RTC observers remain explicit-operation diagnostics.
`OnSystemClockSynchronized` is emitted only on entry into that state; synchronization
before/after values are equal because no step occurs. `Update` services due
callbacks and reliability transitions, including expiry first noticed by a getter.
Scheduled callback capacity is fixed by `ESPRESSIO_TIMING_MAX_CALLBACKS`; arbitrary
callback exceptions are reported then propagated. Clock getters never notify,
even when returning an expired reliability label.

Build native tests with `cmake -S tests -B build
-DESPRESSIO_SYSTEM_SOURCE_DIR=/path/to/ESPressio-System`, then `cmake --build build`
and `ctest --test-dir build --output-on-failure`. The suite retains generic clocks,
provider failures and concurrency while replacing obsolete synchronization tests.
See [K1/K2 validation](docs/K1_K2_VALIDATION.md) for classification and evidence.
