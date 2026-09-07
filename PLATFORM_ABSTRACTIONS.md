# Platform Abstractions Audit Trail

This file records changes made during the platform-abstraction tranche tracked by issue #29.

## 2026-09-07

- Added `Timing::MonotonicClock<TTime>`, a non-settable clock facade intended for elapsed-time scheduling, timeout and cadence consumers which must remain independent of distributed/System Clock correction.
- `MonotonicClock` consumes Timing's process-wide `HighResolutionTimeSource` by default; it does not allocate or own one counter per clock or per consuming thread.
- Default source resolution is lazy on first read so global/static scheduler object construction can occur before platform-provider installation without prematurely constructing the shared high-resolution source.
- On ESP32, `ESP32Platform::InstallSystemProviders()` installs the existing `System::Clock::IHighResolutionCounterProvider` concrete backed by ESP-IDF GPTimer. Timing's shared `HighResolutionTimeSource` therefore owns one GPTimer counter when provider installation precedes the first source read.
- `SystemClock` and `MonotonicClock` may consume the same raw high-resolution source while exposing deliberately different semantics: SystemClock can step/slew/drift-correct its public timeline; MonotonicClock exposes the unchanged elapsed physical timeline.
- Added a host regression which shares one raw source between a SystemClock facade and MonotonicClock, steps SystemClock forward and backward, and verifies the monotonic timeline changes only when raw source ticks advance.

## 2026-08-27

- Created working branch `feature/29-platform-clock-abstractions` from `main`; no tranche changes are committed directly to `main`.
- Removed direct ESP-IDF GPTimer ownership and driver calls from `GPTimerTimeSource`.
- Reworked the historical `GPTimerTimeSource` as a compatibility adapter over `System::Clock::IHighResolutionCounter`.
- Removed `esp_err_t` from the GPTimer compatibility clock API; initialization now reports `System::PlatformResult`.
- Reworked the default `HighResolutionTimeSource` so its dedicated-counter path comes from the System high-resolution counter provider and its fallback path comes from the System monotonic clock.
- Removed direct `esp_timer_get_time()`, Arduino `micros()` and host `std::chrono` selection logic from Timing's reusable source implementation. Platform selection is now performed by System/provider installation.
- Retained `GetIsUsingGPTimer()` as a compatibility name, backed by the generic high-resolution-counter state; new code should use `GetIsUsingHighResolutionCounter()`.
- Added ESPressio-System as a direct dependency and removed Arduino-only package metadata.
- Migrated platform-clock includes to the collision-free `ESPressio_SystemPlatformClock.hpp` header.
- Added `ESPressio_TimingSystemClock.hpp` as the canonical unambiguous include for the disciplined `ESPressio::Timing::SystemClock` API. Cross-library code that needs Timing's system timeline should use this header rather than relying on which package wins resolution of the historical `ESPressio_SystemClock.hpp` filename.
- Retained the historical Timing `ESPressio_SystemClock.hpp` for source compatibility. ESPressio-System's compatibility header now exposes its platform clock contract and, when Timing is present, forwards to `ESPressio_TimingSystemClock.hpp` so legacy include ordering does not silently hide one API.
- Updated README architecture guidance to describe System/provider ownership, ESPressio-ESP32 target implementation, canonical Timing/System clock headers, and the compatibility status of `GPTimerTimeSource` without presenting the working branch as an already-published release.
- Enabled CI on feature/optimisation/bugfix branches and added explicit System dependency coverage. Host, ordinary ESP32, and Serializable-time integration validation have completed successfully on the platform-abstraction branch.

## Boundary

ESPressio-Timing owns clock, stopwatch, synchronization and time-representation semantics, including the shared monotonic clock facade consumed by precision scheduling. ESPressio-System owns the primitive monotonic-clock and high-resolution-counter capabilities. ESPressio-ESP32 owns the `esp_timer` and ESP-IDF GPTimer implementations.
