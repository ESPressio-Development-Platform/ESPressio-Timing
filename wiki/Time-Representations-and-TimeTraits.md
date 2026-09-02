# Time Representations and TimeTraits

Timing algorithms operate in a raw nanosecond tick domain while public clock values are selected through `TTime` and `TimeTraits<TTime>`.

The default public type is an ESPressio Units nanosecond Time value.

A consumer can select another compatible representation, including a Serializable Unit time type, without making Timing itself depend on ESPressio Serializable.

```cpp
using SerializableClockTime =
    Units::SerializableNanoSeconds<uint64_t>;

StopwatchClock<SerializableClockTime> stopwatch;
```

## TimeTraits role

`TimeTraits<TTime>` converts between the internal nanosecond representation and the public `TTime` value while preserving range/conversion semantics.

## Dependency rule

Only the consuming project selecting a Serializable time type needs the corresponding Serializable Units dependency. The clock algorithms remain representation-neutral.

## Custom types

A new public time type can integrate by satisfying the `TimeTraits` contract rather than by duplicating clock algorithms. See [Custom Time Representations](Custom-Time-Representations).