# ESPressio Dependency Chart — Serializable 0.11.2 Cascade

![ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.svg)

Arrows point from a consuming library to the library it consumes. **Required** dependencies are part of the normal library contract; **opt-in** dependencies are introduced only by the corresponding integration/header.

## Current / active generation

```text
Observable    3.0.2
Serializable  0.11.2
Units         0.2.6
Timing        2.2.7   (this release)
Threads       3.1.5   -> next: 3.1.6
Event         6.0.1   -> planned: 6.0.2
Command       1.0.1   -> planned: 1.0.2
Security      0.4.0   -> planned: 0.4.1
Persistence   0.3.0   -> planned: 0.3.1
Sockets       0.7.1   -> planned: 0.7.2
ESP-Now       0.8.1   -> planned: 0.8.2
WiFi          0.1.0   -> unreleased 0.2.0 work in progress
Serial        0.8.0   -> planned: 0.8.1
```

## Required dependencies

```text
Observable 3.0.2
    -> none

Serializable 0.11.2
    -> none

Units 0.2.6
    -> none

Timing 2.2.7
    -> Units >= 0.2.6 < 1.0.0
    -> Observable >= 3.0.2 < 4.0.0

Threads 3.1.5
    -> Timing >= 2.2.5 < 3.0.0
    -> Observable >= 3.0.2 < 4.0.0

Event 6.0.1
    -> Threads >= 3.1.5 < 4.0.0
    -> Timing >= 2.2.5 < 3.0.0
    -> Observable >= 3.0.2 < 4.0.0

Command 1.0.1
    -> Observable >= 3.0.2 < 4.0.0

Security 0.4.0
    -> Observable >= 3.0.2 < 4.0.0

Sockets 0.7.1
    -> Observable >= 3.0.2 < 4.0.0

ESP-Now 0.8.1
    -> Timing >= 2.2.5 < 3.0.0
    -> Observable >= 3.0.2 < 4.0.0

WiFi 0.1.0
    -> Observable >= 3.0.2 < 4.0.0
    -> Serializable >= 0.11.0 < 1.0.0

Serial 0.8.0
    -> none in the core package
```

## Opt-in integrations

```text
Units
    - - -> Serializable >= 0.11.2 < 1.0.0
            Serializable Unit variants

Threads
    - - -> Units Serializable Unit variants
            Serializable PrecisionThread time/frequency traits

Event
    - - -> Serializable >= 0.10.3 < 1.0.0
            Serializable Events / Event Transport

Command
    - - -> Event >= 6.0.1 < 7.0.0

Security
    - - -> Event >= 6.0.1 < 7.0.0

Persistence
    - - -> Serializable >= 0.11.0 < 1.0.0
    - - -> Security >= 0.4.0 < 1.0.0

Sockets
    - - -> Event >= 6.0.1 < 7.0.0
    - - -> Command >= 1.0.1 < 2.0.0
    - - -> Security >= 0.3.1 < 1.0.0
    - - -> Timing >= 2.2.5 < 3.0.0

ESP-Now
    - - -> Event >= 6.0.1 < 7.0.0
    - - -> Command >= 1.0.1 < 2.0.0
    - - -> Security >= 0.3.1 < 1.0.0

WiFi 0.2.0 work
    - - -> Event
    - - -> Command
    - - -> Persistence
    - - -> Security

Serial
    - - -> Serializable >= 0.11.0 < 1.0.0
    - - -> Timing >= 2.2.5 < 3.0.0
    - - -> Threads >= 3.1.5 < 4.0.0
    - - -> Event >= 6.0.1 < 7.0.0
    - - -> Command >= 1.0.1 < 2.0.0
    - - -> Security >= 0.4.0 < 1.0.0
    - - -> Sockets >= 0.7.1 < 1.0.0
    - - -> ESP-Now >= 0.8.1 < 1.0.0
    - - -> WiFi >= 0.1.0 < 1.0.0
```

Timing deliberately has **no direct Serializable edge**. A Serializable Timing public representation is obtained by selecting a Serializable Unit type as `TTime`; Timing 2.2.7 CI validates that path against Units 0.2.6 and Serializable 0.11.2.

## Active propagation order

```text
Serializable 0.11.2
    -> Units 0.2.6
    -> Timing 2.2.7
    -> Threads 3.1.6
    -> Event 6.0.2
    -> Command 1.0.2 / Security 0.4.1
    -> Persistence 0.3.1 / Sockets 0.7.2 / ESP-Now 0.8.2
    -> WiFi 0.2.0
    -> Serial 0.8.1
```

Serial remains terminal/downstream. Tree remains standalone.
