# Clock Discipline and Slewing

The default synchronization adjustment mode is `SlewOnly`: accepted phase error is removed gradually rather than abruptly rebasing the public System Clock.

This preserves monotonic progression and is appropriate while deadline-driven consumers are running.

## Adjustment modes

- `SlewOnly` — correct phase gradually at the configured maximum slew rate.
- `StepIfUnsynchronized` — permit an initial hard step while acquiring/bootstrap is incomplete, then return to normal discipline.
- `StepAlways` — allow immediate steps on accepted samples; use cautiously because the public timeline may move forwards or backwards.

The default maximum phase slew rate in the current baseline is 500 ppm and is configurable.

## Drift estimation

After sufficient accepted samples and once significant phase slewing has settled, Timing estimates residual relative clock-rate error in parts per million and applies continuous rate correction between synchronization exchanges.

Drift learning is suspended during material phase slew so deliberate phase correction is not misinterpreted as oscillator drift.

## Configuration

`ClockSynchronizationConfig` controls maximum round-trip delay, slew/drift limits, filter weights, drift-learning thresholds/intervals, synchronization tolerance, minimum samples and maximum sample age.

## Observer semantics

A synchronization observer reports the **actual immediate public clock difference**. In `SlewOnly`, accepting a +2 ms offset can produce an immediate difference of zero because the correction is pending and applied gradually.