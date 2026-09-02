# Clock Synchronization

Timing provides transport-independent two-way clock synchronization through `IClockSynchronizationTarget<ClockTick>`.

A transport captures four timestamps:

```text
Local                         Remote
T1 request transmit  -------->
                      <-------- T2 request receive
                      <-------- T3 response transmit
T4 response receive
```

and submits a `ClockSynchronizationSample`.

Timing computes the standard estimates:

```text
round-trip delay = (T4 - T1) - (T3 - T2)
clock offset     = ((T2 - T1) + (T3 - T4)) / 2
```

## Transport neutrality

Timing contains no ESP-NOW, WiFi, UDP, Ethernet, CAN, LoRa, MAC-address, peer-election or packet-format concepts. The transport owns timestamp capture/exchange; Timing owns sample validation and clock discipline.

## State

Synchronization state progresses through `Unsynchronized`, `Acquiring`, and `Synchronized`, with status including measured/filtered offset, pending/applied correction, round-trip delay, drift estimate, accepted/rejected counts and last accepted sample time.

A synchronized clock can become stale when accepted samples stop arriving within the configured maximum age.

## Rejection

Malformed samples and samples exceeding the configured maximum round-trip delay are rejected before influencing the disciplined clock.