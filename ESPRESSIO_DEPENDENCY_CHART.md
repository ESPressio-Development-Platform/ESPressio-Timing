# ESPressio Dependency Chart

![ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.png)

## ESPressio Timing 2.2.3

Timing has two required ESPressio dependencies:

```text
ESPressio Timing 2.2.3
    -> ESPressio Units >= 0.2.2 < 1.0.0
    -> ESPressio Observable >= 3.0.1 < 4.0.0
```

Timing deliberately has **no direct ESPressio Serializable dependency**.
Applications may choose Serializable time representations through the opt-in
Serializable Unit types supplied by Units 0.2.2:

```text
Timing
    -> Units
        - - -> Serializable >= 0.10.1 < 1.0.0
```

This keeps the dependency cascade one-way and avoids making serialization part
of the Timing core.

## Current coordinated ecosystem

```text
Observable 3.0.1
Serializable 0.10.1
Units 0.2.2
    |
    v
Timing 2.2.3
    |
    +--> Threads 3.1.3
    |
    +--> ESP-Now 0.5.1
              |
              - - -> Event 5.8.1 (Event transport integration)

Threads 3.1.3
    |
    v
Event 5.8.1

Serial 0.5.1 remains a downstream opt-in diagnostics/operator layer.
```

Security 0.2.0, Command 0.3.0 and Sockets 0.5.0 remain separate branches of
the integration graph except where their explicit opt-in adapters are selected.

## Dependency-direction rule

A dependency should cascade downstream. An upstream/foundational library should
not depend back on a downstream implementation merely to host integration code.

The known Event/ESP-Now reciprocal optional relationship is therefore considered
an architectural exception to resolve:

```text
ESP-Now - - -> Event
    ESPNowEventTransport

Event - - -> ESP-Now
    ESPNowTransportEventBridge
```

The preferred resolution is to keep Event upstream and transport-neutral and
move the ESP-Now-specific Observer-to-Event bridge downstream into ESP-Now's
Event integration (or a dedicated downstream integration package).

No new reciprocal dependency should be introduced.
