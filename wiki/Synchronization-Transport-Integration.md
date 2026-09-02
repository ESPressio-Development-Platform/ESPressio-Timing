# Synchronization Transport Integration

A transport integrates with Timing through `IClockSynchronizationTarget<ClockTick>` rather than by depending on a concrete `SystemClock<TTime>` facade.

## Transport responsibilities

The transport must capture and carry the four synchronization timestamps with the best timing fidelity its medium can provide, correlate request/response exchanges, and submit a complete `ClockSynchronizationSample`.

It also decides peer addressing, packet layout, retry policy, master/peer selection and link lifecycle.

## Timing responsibilities

Timing validates the sample, estimates offset and round-trip delay, filters samples, manages acquisition/synchronized state, slews phase, learns drift and applies rate correction.

## Timestamp domain

Synchronization uses the raw nanosecond clock domain, making it independent of the public Units/Serializable time representation.

## Extension guidance

A new radio/network transport should adapt its packet/peer model to this interface. Do not add transport names, packet structures or peer identifiers to Timing.

## Testing

Use deterministic synthetic timestamp exchanges to verify sign conventions and capture ordering before hardware/network jitter is introduced.