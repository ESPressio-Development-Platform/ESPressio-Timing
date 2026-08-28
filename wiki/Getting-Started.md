# Getting Started

Timing's public clock APIs use ESPressio Units for their default time representation while keeping raw timing arithmetic in a nanosecond tick domain.

For the disciplined System Clock:

```cpp
#include <ESPressio_TimingSystemClock.hpp>

using namespace ESPressio::Timing;

auto& clock = SystemClock<>::GetInstance();
auto now = clock.GetTime();
```

For a stopwatch:

```cpp
StopwatchClock<> stopwatch;
stopwatch.Start();
auto elapsed = stopwatch.GetTime();
```

## Platform setup

Timing no longer selects ESP-IDF GPTimer, `esp_timer`, Arduino `micros()` or host fallbacks directly. Install the appropriate ESPressio System clock/high-resolution-counter providers before Timing first requires them.

## One System timeline

Different `SystemClock<TTime>` specializations are typed facades over the same underlying `SystemClockCore`; selecting another public time type does not create another system timeline.

## Next steps

- [System Clock](System-Clock)
- [Clock Synchronization](Clock-Synchronization)
- [Time Representations and TimeTraits](Time-Representations-and-TimeTraits)
- [High Resolution Sources](High-Resolution-Sources)