# Changelog

All notable changes to this project are documented in this file.

The structure follows the principles of [Keep a
Changelog](https://keepachangelog.com/en/1.1.0/) and [Semantic
Versioning](https://semver.org/).

> **Historical note:** This changelog was reconstructed retrospectively
> from published GitHub Releases, tags, release notes, repository
> history, and the documented public API. Where an historical release
> had little or no release-note detail, the entry is intentionally terse
> rather than inferring unsupported intent.

## \[2.2.0\] - 2026-08-18

### Added

-   Added first-class ESPressio Observable support throughout Timing.
-   Added System Clock Observer notifications for synchronization
    lifecycle and state changes.
-   Added rich before/after synchronization information, including
    clock-difference data.
-   Added notifications for synchronization reset and other logical
    clock operations where observation is meaningful.

### Changed

-   Made ESPressio Observable 3.x part of the Timing notification
    architecture.
-   Preserved synchronous Observer semantics so higher-level libraries
    can optionally bridge notifications into asynchronous Events.

## \[2.1.0\] - 2026-08-18

### Added

-   Added transport-independent System Clock synchronization and clock
    discipline.
-   Added synchronization target/sample abstractions suitable for
    ESP-NOW, UDP, TCP, WebSocket, and future transports.
-   Added four-timestamp request/response synchronization support.
-   Added synchronization state and correction policy infrastructure.

### Changed

-   Kept synchronization calculations and System Clock discipline inside
    Timing while leaving message transport to dedicated communication
    libraries.

## \[2.0.0\] - 2026-08-18

### Added

-   Added generic public time representations through
    `TimeTraits<TTime>`.
-   Added support for custom Timing-compatible public time
    representations.
-   Added independently configurable raw tick/storage representation.
-   Added typed `SystemClock<TTime>` facades over a single shared System
    Clock core.

### Changed

-   Separated internal nanosecond/tick arithmetic from public Unit
    representation.
-   Redesigned System Clock singleton handling so different template
    representations do not create separate system timelines.
-   Enabled ordinary and Serializable ESPressio Unit time types without
    introducing a direct Serializable dependency.

## \[1.1.0\] - 2026-08-13

### Added

-   Initial public release of ESPressio Timing.
-   Added the original clock, stopwatch, RTC/time-source, System Clock,
    and callback-scheduling foundation.
