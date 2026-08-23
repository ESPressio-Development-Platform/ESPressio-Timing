# ESPressio Dependency Chart — Serializable 0.11.3 Cascade

![ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.svg)

Arrows point from a consuming library to the library it consumes. **Required** dependencies are part of the normal library contract; **opt-in** dependencies are introduced only by the corresponding integration/header.

## Current / active generation

```text
Observable    3.0.2
Serializable  0.11.3
Units         0.2.7
Timing        2.2.8   (this release)
Threads       3.1.6   -> next: 3.1.7
Event         6.0.2
Command       1.0.2
Security      0.4.1
Persistence   0.3.1
Sockets       0.7.2
ESP-Now       0.8.2
WiFi          0.2.0   -> merged, awaiting released-dependency verification
Serial        0.8.0   -> terminal cascade target
```

## Required dependencies

```text
Observable 3.0.2
    -> none

Serializable 0.11.3
    -> none

Units 0.2.7
    -> none

Timing 2.2.8
    -> Units >= 0.2.7 < 1.0.0
    -> Observable >= 3.0.2 < 4.0.0

Threads 3.1.6
    -> Timing >= 2.2.7 < 3.0.0
    -> Observable >= 3.0.2 < 4.0.0

Event 6.0.2
    -> Threads >= 3.1.6 < 4.0.0
    -> Timing >= 2.2.7 < 3.0.0
    -> Observable >= 3.0.2 < 4.0.0

Command 1.0.2
    -> Observable >= 3.0.2 < 4.0.0

Security 0.4.1
    -> Observable >= 3.0.2 < 4.0.0

Sockets 0.7.2
    -> Observable >= 3.0.2 < 4.0.0

ESP-Now 0.8.2
    -> Timing >= 2.2.7 < 3.0.0
    -> Observable >= 3.0.2 < 4.0.0

WiFi 0.2.0
    -> Observable >= 3.0.2 < 4.0.0
    -> Serializable >= 0.11.2 < 1.0.0
    -> Threads >= 3.1.6 < 4.0.0

Serial 0.8.0
    -> none in the core package
```

## Opt-in integrations

```text
Units
    - - -> Serializable >= 0.11.3 < 1.0.0
            Serializable Unit variants

Threads
    - - -> Units Serializable Unit variants
            Serializable PrecisionThread time/frequency traits

Event
    - - -> Serializable
            Serializable Events / Event Transport

Command
    - - -> Event

Security
    - - -> Event

Persistence
    - - -> Serializable
    - - -> Security

Sockets
    - - -> Event
    - - -> Command
    - - -> Security
    - - -> Timing

ESP-Now
    - - -> Event
    - - -> Command
    - - -> Security

WiFi
    - - -> Event
    - - -> Command
    - - -> Persistence
    - - -> Security

Serial
    - - -> Serializable
    - - -> Timing
    - - -> Threads
    - - -> Event
    - - -> Command
    - - -> Security
    - - -> Sockets
    - - -> ESP-Now
    - - -> WiFi
```

Timing deliberately has **no direct Serializable edge**. A Serializable Timing public representation is obtained by selecting a Serializable Unit type as `TTime`; Timing 2.2.8 CI validates that path against Units 0.2.7 and Serializable 0.11.3.

## Active propagation order

```text
Serializable 0.11.3
    -> Units 0.2.7
    -> Timing 2.2.8
    -> Threads 3.1.7
    -> Event
    -> Command / Security
    -> Persistence / Sockets / ESP-Now
    -> WiFi 0.2.0
    -> Serial
```

Serial remains terminal/downstream. Tree remains standalone.
