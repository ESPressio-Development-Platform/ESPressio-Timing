# ESPressio Timing

Generic high-resolution clock, stopwatch, RTC, and distributed
clock-synchronization abstractions.

## Latest Stable Version

**2.2.0**

## ESPressio Development Platform

ESPressio is a collection of discrete, composable component libraries
built around a common development ethos:

-   **Light-weight** --- minimise memory consumption and runtime
    overhead without sacrificing correctness.
-   **Ease of use** --- provide strongly typed, developer-friendly
    abstractions over lower-level facilities.
-   **Object-oriented** --- a type for everything, and everything in a
    type.
-   **SOLID** --- favour focused responsibilities, extensibility,
    substitutable abstractions, narrow interfaces, and dependency
    inversion wherever practical on embedded C++ platforms.

## License

Licensed under the **Apache License 2.0**. See [LICENSE](LICENSE).

## ESPressio Library Dependencies

ESPressio is designed as a modular ecosystem of independently useful
libraries, with required dependencies kept explicit and optional
integrations introduced only when the corresponding functionality is
selected.

For a complete overview of required dependencies, opt-in dependencies,
and the overall hierarchy, see:

**[ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.md)**

-   **Solid relationships** represent required ESPressio dependencies.
-   **Dashed relationships** represent opt-in dependencies introduced
    only by the corresponding feature, integration, type, or header.

### Required ESPressio dependencies

-   **ESPressio Units \>= 0.2.0**
-   **ESPressio Observable \>= 3.0.0**

Serializable is not a direct Timing dependency. Serializable time
representations are supplied by the opt-in Serializable variants in
Units.

## Generic time representation

Timing 2.x is generic over its public time representation. The default
is `Timing::DefaultClockTime`, while another compatible Units
representation can be selected at instantiation time.

This permits ordinary and Serializable time representations without
creating two Timing implementations.

## System Clock

There is one underlying System Clock.

Typed `SystemClock<TTime>` facades expose the same singleton clock using
the selected representation:

``` text
one System Clock core
    +--> SystemClock<DefaultClockTime>
    +--> SystemClock<Serializable time>
    +--> SystemClock<other compatible time>
```

## Clock synchronization

Timing owns the transport-independent synchronization model and clock
discipline.

Concrete communication libraries only exchange synchronization messages:

``` text
Timing
    -> synchronization semantics / correction

ESP-Now
    -> ESP-NOW transport

Sockets
    -> UDP/TCP/WebSocket/SNTP integration
```

Where supported, the synchronization sample follows the four-timestamp
model:

``` text
Client                         Authority

T1 ---- request -------------> T2
                               T3
T4 <--- response -------------
```

Timing receives the completed sample and calculates/applies the
correction.

## Observable notifications

Timing 2.2 applies ESPressio Observable to logical clock operations,
especially System Clock synchronization.

Synchronization notifications expose relevant before/after state,
including the difference between pre- and post-synchronization clock
values.

ESPressio Event may optionally bridge these synchronous callbacks into
asynchronous Timing Events through `SystemClockEventBridge`.

Timing itself does not depend on Event.

## Stopwatch, RTC and time sources

The library retains its high-resolution stopwatch/clock abstractions and
extensible RTC/external-source model while presenting strongly typed
time values at the public API boundary.

## Design goals

-   One System Clock core.
-   Generic strongly typed time representation.
-   Transport-neutral synchronization.
-   Observable lifecycle/state changes.
-   No mandatory Serializable dependency.
-   No Event dependency.
-   Concrete transport mechanisms implemented by communication
    libraries.
