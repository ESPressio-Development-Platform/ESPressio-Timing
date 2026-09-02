# Custom Time Representations

A custom public time representation integrates through `TimeTraits<TTime>`.

The traits contract translates between Timing's internal nanosecond/tick domain and the public C++ value without changing the clock algorithms.

## Requirements

A custom representation should provide deterministic conversion to/from nanoseconds, define range/overflow behaviour, preserve the intended signed/unsigned semantics, and remain a value type suitable for the clock interface that exposes it.

## No algorithm duplication

Do not create a parallel `CustomSystemClock` merely to change the returned value type. `SystemClock<TTime>` already provides a typed facade over the shared core.

## Serializable values

If the public value is serializable, that dependency belongs to the consumer/type integration. Timing itself must remain independent of ESPressio Serializable.

## Testing

Exercise zero, representative magnitudes, maximum/minimum supported range, conversion round trips and all clock APIs expected to expose the custom type.