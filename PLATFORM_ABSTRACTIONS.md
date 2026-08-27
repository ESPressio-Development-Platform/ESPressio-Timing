# Platform Abstractions Audit Trail

This file records changes made during the platform-abstraction tranche tracked by issue #29.

## 2026-08-27

- Created working branch `feature/29-platform-clock-abstractions` from `main`; no tranche changes are committed directly to `main`.
- Removed direct ESP-IDF GPTimer ownership and driver calls from `GPTimerTimeSource`.
- Reworked the historical `GPTimerTimeSource` as a compatibility adapter over `System::Clock::IHighResolutionCounter`.
- Removed `esp_err_t` from the GPTimer compatibility clock API; initialization now reports `System::PlatformResult`.
- Reworked the default `HighResolutionTimeSource` so its dedicated-counter path comes from the System high-resolution counter provider and its fallback path comes from the System monotonic clock.
- Removed direct `esp_timer_get_time()`, Arduino `micros()` and host `std::chrono` selection logic from Timing's reusable source implementation. Platform selection is now performed by System/provider installation.
- Retained `GetIsUsingGPTimer()` as a compatibility name, backed by the generic high-resolution-counter state; new code should use `GetIsUsingHighResolutionCounter()`.
- Added ESPressio-System as a direct dependency and removed Arduino-only package metadata.
- Migrated System-clock includes to the canonical `ESPressio_SystemClock.hpp` header. This avoids the historical package-level collision with Timing's own `ESPressio_Clock.hpp` when System and Timing appear in the same PlatformIO dependency graph.

## Boundary

ESPressio-Timing owns clock, stopwatch, synchronization and time-representation semantics. ESPressio-System owns the primitive monotonic-clock and high-resolution-counter capabilities. ESPressio-ESP32 owns the `esp_timer` and ESP-IDF GPTimer implementations.
