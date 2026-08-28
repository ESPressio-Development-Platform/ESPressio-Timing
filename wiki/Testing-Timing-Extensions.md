# Testing Timing Extensions

Timing tests should separate deterministic timing-domain algorithms from target clock-provider integration.

## Clock semantics

Test stopwatch lifecycle, set/read behaviour, resolution, monotonicity, shared System Clock facade/core semantics, scheduling and observer transitions.

## Synchronization

Use synthetic four-timestamp samples to verify offset/round-trip calculations, malformed/high-delay rejection, acquisition/synchronized/stale state, filtering, phase slew, startup stepping, drift estimation and rate correction.

## Discipline safety

Verify `SlewOnly` never creates an immediate public clock jump and remains monotonic. Test explicit step modes separately, including their interaction with synchronization reset/state.

## Public representations

Run clock operations through the default Units type and custom/Serializable-compatible `TimeTraits` paths without making Serializable a core requirement.

## Platform providers

Hardware-specific monotonic/high-resolution-counter conformance belongs with the System/platform provider tests; Timing should test against the portable provider contracts.