# ESPressio Dependency Chart

![ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.png)

## ESPressio Timing 2.2.4

Timing has two required ESPressio dependencies:

```text
ESPressio Timing 2.2.4
    -> ESPressio Units >= 0.2.3 < 1.0.0
    -> ESPressio Observable >= 3.0.1 < 4.0.0
```

Timing deliberately has **no direct ESPressio Serializable dependency**.
Applications may choose Serializable time representations through the opt-in
Serializable Unit types supplied by Units 0.2.3:

```text
Timing
    -> Units
        - - -> Serializable >= 0.10.2 < 1.0.0
```

This keeps the dependency cascade one-way and avoids making serialization part
of the Timing core.

## Current coordinated ecosystem

```text
Observable 3.0.1
Serializable 0.10.2
Units 0.2.3 (candidate until released)
    |
    v
Timing 2.2.4
    |
    +--> Threads 3.1.4 (planned downstream refresh)
    |
    +--> ESP-Now 0.5.2 (planned downstream refresh)

Event 5.8.2 is the planned downstream convergence release after Threads and
ESP-Now are refreshed.

Serial 0.5.1 remains the final downstream diagnostics/operator release
candidate.
```

Security 0.2.0, Command 0.3.0 and Sockets 0.5.0 remain separate branches of
the integration graph except where their explicit opt-in adapters are selected.

## Dependency-direction rule

A dependency should cascade downstream. An upstream/foundational library should
not depend back on a downstream implementation merely to host integration code.

The known reciprocal optional relationships remain architectural exceptions to
resolve separately:

```text
ESP-Now - - -> Event
    ESPNowEventTransport

Event - - -> ESP-Now
    ESPNowTransportEventBridge
```

and:

```text
Sockets - - -> Event
    socket Event transports

Event - - -> Sockets
    SocketWorkerEventBridge
    SocketSecuritySessionEventBridge
```

The preferred resolution is to keep Event upstream and transport-neutral and
move transport-specific Observer-to-Event bridges downstream alongside each
transport library's Event integration (or a dedicated downstream integration
package).

No new reciprocal dependency should be introduced.
