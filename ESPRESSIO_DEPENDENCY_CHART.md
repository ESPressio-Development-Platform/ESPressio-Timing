# ESPressio Dependency Chart — Current Released Generation

![ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.svg)

This document records the completed Serializable 0.11.3 cascade and the current released ESPressio dependency generation. Arrows point from a consuming library to the library it consumes.

## Released generation

```text
Observable    3.0.2
Serializable  0.11.3
Units         0.2.7
Timing        2.2.8
Threads       3.1.7
Event         6.0.3
Command       1.0.3
Security      0.4.2
Persistence   0.3.2
Sockets       0.7.3
ESP-Now       0.8.3
WiFi          0.2.0
Serial        0.8.1
```

## Timing dependency position

```text
Timing 2.2.8
    -> Units main
    -> Observable main
```

Timing deliberately has **no direct Serializable dependency**. Serializable Timing representations are obtained by selecting Serializable Unit types as `TTime`; during the release restructuring that path is validated against Units `main` and Serializable `main`.

## Completed propagation

```text
Serializable 0.11.3
    -> Units 0.2.7
    -> Timing 2.2.8
    -> Threads 3.1.7
    -> Event 6.0.3
    -> Command 1.0.3 / Security 0.4.2
    -> Persistence 0.3.2 / Sockets 0.7.3 / ESP-Now 0.8.3
    -> WiFi 0.2.0
    -> Serial 0.8.1
```

## Dependency-direction invariants

- Timing owns clock/time algorithms and depends on Units + Observable.
- Threads may consume Timing; Timing must not depend on Threads.
- Event may consume Timing; Timing must not depend on Event.
- Serializable representations remain opt-in through Units rather than becoming a Timing core dependency.
- Serial remains terminal/downstream.
- ESPressio Tree remains standalone.
