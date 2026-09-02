# Platform Clock Boundary

Target-specific clock acquisition belongs below ESPressio Timing.

Timing consumes primitive capabilities from ESPressio System:

- a monotonic nanosecond clock;
- a dedicated high-resolution counter where available.

The target platform package implements those contracts using its SDK/hardware.

## Do not put native timers in Timing

A Timing change must not reintroduce direct ownership of facilities such as ESP-IDF `esp_timer`, GPTimer, Arduino `micros()`, native timer handles, or target SDK error types.

## Why the boundary exists

The same stopwatch, System Clock, synchronization and scheduling algorithms can then execute over any target whose System providers satisfy the semantic capability.

## Includes

Use `ESPressio_SystemPlatformClock.hpp` when implementing against the primitive System platform-clock contract. Use `ESPressio_TimingSystemClock.hpp` for the disciplined Timing System Clock API.

## Compatibility names

Historical GPTimer-named Timing adapters may remain for source compatibility but must continue to delegate to the generic System high-resolution-counter contract rather than becoming a target-specific implementation layer.