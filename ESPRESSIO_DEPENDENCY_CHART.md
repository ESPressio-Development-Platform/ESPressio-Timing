# ESPressio Dependency Chart — Current Released Generation

![ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.svg)

This document records the completed Serializable cascade and the current released ESPressio dependency generation. Arrows point from a consuming library to the library it consumes.

## Released generation

```text
Observable
Serializable
Units
Timing
Threads
Event
Command
Security
Persistence
Sockets
ESP-Now
WiFi
Serial
```

## Timing dependency position

```text
Timing
    -> Units main
    -> Observable main
```

Timing deliberately has **no direct Serializable dependency**. Serializable Timing representations are obtained by selecting Serializable Unit types as `TTime`; during the release restructuring that path is validated against Units `main` and Serializable `main`.

## Completed propagation

```text
Serializable
    -> Units
    -> Timing
    -> Threads
    -> Event
    -> Command / Security
    -> Persistence / Sockets / ESP-Now
    -> WiFi
    -> Serial
```

## Dependency-direction invariants

- Timing owns clock/time algorithms and depends on Units + Observable.
- Threads may consume Timing; Timing must not depend on Threads.
- Event may consume Timing; Timing must not depend on Event.
- Serializable representations remain opt-in through Units rather than becoming a Timing core dependency.
- Serial remains terminal/downstream.
- ESPressio Tree remains standalone.
