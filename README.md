# ESPressio Timing
Timing Components of the Flowduino ESPressio Development Platform.

High-resolution system, stopwatch, and RTC clock abstractions for microcontroller development.

## Latest Stable Version
The latest Stable Version is [1.0.0](https://github.com/Flowduino/ESPressio-Timing/releases/tag/1.0.0). The current source is version 1.1.0.

## Compatibility

The interfaces and clocks use portable Arduino C++11 and ESPressio Units. They may compile on any Arduino target with the required standard-library support, including ESP32, ESP8266, RP2040, SAMD, STM32, Renesas, Teensy, and AVR toolchains.

The default time source selects the highest-resolution monotonic API that can be used portably on each supported build:

- ESP32 uses `esp_timer_get_time()` at one-microsecond resolution.
- Other Arduino targets use `micros()` at the resolution supplied by their core. The 32-bit rollover is extended into a 64-bit tick count while the clock is observed at least once per rollover period.
- Hosted C++ builds use `std::chrono::steady_clock` at its native period.

All public clock values are `ESPressio::Units::Time<uint64_t, Nano>` values. Their context is always `UnitContext::Time`, while their runtime `orderOfMagnitude` represents the clock's actual precision. Applications with a higher-resolution peripheral can implement `ITimeSource` and inject it into `StopwatchClock` or an `RTCClockBase` descendant without changing the clock API.

ESPressio Timing therefore depends on ESPressio Units 0.1.0 or later. ESPressio Units currently requires compiler exception support, even though the Timing implementation itself does not throw exceptions.

## ESPressio Development Platform
The **ESPressio** Development Platform is a collection of discrete (sometimes intra-connected) Component Libraries developed with a particular development ethos in mind.

The key objectives of the ESPressio Development Platform are:
- **Light-weight** - The Components should always strive to optimize memory consumption and operational overhead as much as possible, but not to the detriment of...
- **Ease of Use** - Many of our components serve as Developer-Friendly Abstractions of existing procedural code libraries.
- **Object-Oriented** - A `type` for everything, and everything in a `type`!
- **SOLID**:
- -  > **S**ingle Responsibility Principle (SRP)
    Break your code into smaller, focused components.
- - > **O**pen/Closed Principle (OCP)
    Be open for extension but closed for modification.
- - > **L**iskov Substitution Principle (LSP)
    Be substitutable for the base type without altering correctness.
- - > **I**nterface Segregation Principle (ISP)
    Break interfaces into specific, client-focused ones.
- - > **D**ependency Inversion Principle (DIP)
    Be dependent on abstractions, not concretions.

To the maximum extent possible within the limitations/restrictons/constraints of the C++ langauge, the Arduino platform, and Microcontroller Programming itself, all Component Libraries of the **ESPressio** Development Platform must strive to honour the **SOLID** principles.

## License
ESPressio (and its component libraries, including this one) are subject to the *Apache License 2.0*
Please see the [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE) accompanying this library for full details.

## Namespace
Every type/variable/constant/etc. related to *ESPressio* Timing are located within the `Timing` sub-namespace of the `ESPressio` parent namespace.

The namespace provides the following (*click on any declaration to navigate to more info*):

- `ESPressio::Timing::IClock`
- `ESPressio::Timing::ITimeSource`
- `ESPressio::Timing::HighResolutionTimeSource`
- `ESPressio::Timing::SystemClock`
- `ESPressio::Timing::StopwatchClock`
- `ESPressio::Timing::IRTCClock`
- `ESPressio::Timing::RTCClockBase`

## Platformio.ini
You can quickly and easily add this library to your project in PlatformIO by simply including the following in your `platformio.ini` file:

```ini
lib_deps =
    flowduino/ESPressio-Timing@^1.1.0
```

Alternatively, if you want to use the bleeding-edge (effectively "Developer Integration Testing" or "DIT") sources, you can instead use:

```ini
lib_deps = 
	https://github.com/Flowduino/ESPressio-Timing.git
```
Please note that this will use the very latest commits pushed into the repository, so volatility is possible.

## Time Representation

`ClockTime` is an alias of `ESPressio::Units::Time<uint64_t, Nano>`. It provides a 64-bit value, the `Time` context, and a runtime order of magnitude. For example, the default ESP32 source returns values with `orderOfMagnitude == Micro`, while a seconds-resolution RTC returns values with `orderOfMagnitude == Base`.

Raw internal calculations use nanoseconds. Constants are provided for those calculations and for hardware integrations:

```cpp
NanosecondsPerMicrosecond
NanosecondsPerMillisecond
NanosecondsPerSecond
```

The clock chooses the finest SI magnitude justified by its resolution. `GetResolution()` returns the same Time-unit type, including values such as `1 us`, `4 us`, or `1 s`. Non-decimal resolutions such as 12.5 nanoseconds remain represented in nanoseconds.

All clock implementations share the same `IClock` interface. Code accepting an `IClock&` or `IClock*` can call `GetTime()` identically for `SystemClock`, `StopwatchClock`, and any concrete `RTCClockBase` descendant. For a system or RTC clock, the value is its current timestamp; for a stopwatch, it is the elapsed duration.

## System Clock

`SystemClock` is a singleton monotonic clock with a settable epoch. Setting it changes the mapping between monotonic hardware time and the exposed system time; it does not modify the underlying hardware timer.

```cpp
#include <ESPressio_Timing.hpp>

using namespace ESPressio::Timing;

SystemClock* clock = SystemClock::GetInstance();
clock->SetTime(ClockTime(30, ESPressio::Units::Base));
ClockTime now = clock->GetTime();
```

An application-specific `ITimeSource*` may be supplied to the first `GetInstance(source)` call. Because the system clock is a singleton, the source selected by that first call remains in use for the process lifetime.

Callbacks are stored in a fixed-capacity array and are invoked by `Update()`. This keeps the clock portable and avoids creating a hidden task or relying on a particular interrupt controller. The default capacity is eight and can be changed with `ESPRESSIO_TIMING_MAX_CALLBACKS`.

## Stopwatch Clock

`StopwatchClock` measures elapsed monotonic time at the full resolution of its source. It supports `Start()`, `Stop()`, `Reset()`, `Restart()`, `SetTime()`, and `GetLapTime()`.

```cpp
StopwatchClock stopwatch(true);

// Work to measure...
ClockTime elapsed = stopwatch.GetTime();
Serial.println(elapsed.AsString());
```

## RTC Clock Base

Derive an RTC implementation from `RTCClockBase`, provide `ReadRTC()` and `WriteRTC()`, and specify the RTC's real resolution. Call `Synchronize()` after the device is initialized.

```cpp
class MyRTCClock : public RTCClockBase {
    public:
        MyRTCClock()
            : RTCClockBase(
                ClockTime(1, ESPressio::Units::Base)
            ) { }

    protected:
        bool ReadRTC(ClockTime& time) override {
            // Read the device and return a Time value in its native magnitude.
            return true;
        }

        bool WriteRTC(ClockTime time) override {
            // Convert the timestamp and write it to the device.
            return true;
        }
};
```

For an interrupt-driven RTC, capture the interrupt's exact timestamp and defer `OnRTCInterrupt(exactTime)` to a safe execution context. This avoids bus I/O and non-atomic 64-bit state changes inside the ISR. The no-argument `OnRTCInterrupt()` calls `ReadRTC()` and is intended only when that device operation is safe in the calling context. Between RTC synchronizations, the base extrapolates internally using the injected monotonic source, while returned values retain the RTC's declared precision magnitude.

## Understanding Timing
`Timing` in the context of `ESPressio Timing` refers to a reliable implementation for ensuring precise control and calculation of Time.

The specific implementation used will depend quite heavily on your hardware, and this will similarly affect the maximum *Resolution* of Timing available in your program.

As an example, most ESP32 Micro Controllers (MCUs) are able to leverage the integrated *General Purpose Timer* (or *GPTimer*) to provide extremely precise, hardware-driven Timing with a resolution as low as 1 microsecond (1us).
Conversely, with an Arduino or any other MCU not featuring an integrated high-precision Timing unit, you may need to leverage an oscillating crystal (commonly included with Real-Time Clock (RTC) hardware modules). However, while this will provide you with the means to implement a reliable Precision Timer, the resolution of this Timer may be considerably lower than with the integrated *GPTimer* of the ESP32 MCU. Most RTC units, for example, can only provide precise Timing with a resolution of 32 microseconds (32us), thus 32x lower resolution than the ESP32's *GPTimer*.

The default ESP32 implementation uses `esp_timer`, which provides a stable microsecond monotonic clock across the ESP32 family. Applications requiring a dedicated GPTimer, CPU cycle counter, or another higher-resolution peripheral can expose its native counter through `ITimeSource`; the system and stopwatch abstractions then retain that source's resolution without a platform-specific public API.

## Why is Timing important?
Precision Timing is an extremely common requirement for a broad range of hardware devices (and their corresponding software).

A chronometer, for example, needs to provide extremely precise Timing... as precision Timing is precisely what a chronometer exists to provide.

Beyond the obvious, it's worth considering that the individual processing Threads of a Video Game Engine also require precise Timing. This is for the purpsoe of calculating *Delta Time* (typically defined as the precise amount of Time that has passed between two cycles of a particular Thread). Without accurate *Delta Time*, a Video Game Engine cannot accurately interpolate (or even extrapolate) *State Changes* between cycles. This would present considerable problems when computing, for example, Physics Data within the game's "World."

The particular need for precision Timing in your hardware (and its operating software) are, of course, yours to determine... however, the ESPressio Timing library exists to facilitate your precision Timing needs.
