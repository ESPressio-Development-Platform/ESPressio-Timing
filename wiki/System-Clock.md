# System Clock

`SystemClock<TTime>` is a typed facade over one shared `SystemClockCore`. All public time representations observe the same global device timeline.

```cpp
using SerializableClockTime =
    Units::SerializableNanoSeconds<uint64_t>;

auto& ordinary = SystemClock<>::GetInstance();
auto& serializable =
    SystemClock<SerializableClockTime>::GetInstance();
```

Setting or synchronizing through one facade changes the same underlying clock observed through the other.

## Canonical include

Cross-library code should include:

```cpp
#include <ESPressio_TimingSystemClock.hpp>
```

This avoids ambiguity with the primitive ESPressio System platform-clock header.

## Hard rebasing

`SetTime()` is an explicit hard rebase. It resets synchronization/discipline state because previously measured phase and drift estimates are no longer valid.

## Scheduling

The shared core also owns callbacks in raw nanosecond ticks. A callback registered through one typed facade can therefore be serviced through another facade's `Update()` call.

## Synchronization

The same core implements `IClockSynchronizationTarget<ClockTick>` so transport integrations do not depend on a particular public `TTime` specialization.