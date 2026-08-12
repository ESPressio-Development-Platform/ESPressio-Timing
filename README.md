# ESPressio Timing
Threading Components of the Flowduino ESPressio Development Platform.

Light-weight and easy-to-use Threading for your Microcontroller development work.

## Latest Stable Version
The latest Stable Version is [1.0.0](https://github.com/Flowduino/ESPressio-Timing/releases/tag/1.0.0).

## Compatibility

The interfaces in this repository use platform-neutral C++ and may compile on any target with C++11 support, including ESP32, ESP8266, RP2040, SAMD, STM32, Renesas, Teensy, and AVR toolchains.

However, the current `SystemClock` methods are placeholders and the repository has no Arduino or PlatformIO library manifest. No hardware-backed clock or timer is presently implemented, so the library should not yet be described as functionally compatible with any microcontroller. Device-specific compatibility can be declared once concrete timer implementations and build verification are added.

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
- [`ESPressio::Timing::ITimer](#itimer)

## Platformio.ini
You can quickly and easily add this library to your project in PlatformIO by simply including the following in your `platformio.ini` file:

```ini
lib_deps =
    flowduino/ESPressio-Timing@^1.0.0
```

Alternatively, if you want to use the bleeding-edge (effectively "Developer Integration Testing" or "DIT") sources, you can instead use:

```ini
lib_deps = 
	https://github.com/Flowduino/ESPressio-Timing.git
```
Please note that this will use the very latest commits pushed into the repository, so volatility is possible.

## Understanding Timing
`Timing` in the context of `ESPressio Timing` refers to a reliable implementation for ensuring precise control and calculation of Time.

The specific implementation used will depend quite heavily on your hardware, and this will similarly affect the maximum *Resolution* of Timing available in your program.

As an example, most ESP32 Micro Controllers (MCUs) are able to leverage the integrated *General Purpose Timer* (or *GPTimer*) to provide extremely precise, hardware-driven Timing with a resolution as low as 1 microsecond (1us).
Conversely, with an Arduino or any other MCU not featuring an integrated high-precision Timing unit, you may need to leverage an oscillating crystal (commonly included with Real-Time Clock (RTC) hardware modules). However, while this will provide you with the means to implement a reliable Precision Timer, the resolution of this Timer may be considerably lower than with the integrated *GPTimer* of the ESP32 MCU. Most RTC units, for example, can only provide precise Timing with a resolution of 32 microseconds (32us), thus 32x lower resolution than the ESP32's *GPTimer*.

The intended ESPressio Timing design includes implementations for both the *GPTimer* on the ESP32 and a generic interrupt-driven timer with configurable precision. Those hardware-backed implementations are not present in the current repository; at present it provides interfaces and placeholder clock types from which concrete platform integrations can be developed.

## Why is Timing important?
Precision Timing is an extremely common requirement for a broad range of hardware devices (and their corresponding software).

A chronometer, for example, needs to provide extremely precise Timing... as precision Timing is precisely what a chronometer exists to provide.

Beyond the obvious, it's worth considering that the individual processing Threads of a Video Game Engine also require precise Timing. This is for the purpsoe of calculating *Delta Time* (typically defined as the precise amount of Time that has passed between two cycles of a particular Thread). Without accurate *Delta Time*, a Video Game Engine cannot accurately interpolate (or even extrapolate) *State Changes* between cycles. This would present considerable problems when computing, for example, Physics Data within the game's "World."

The particular need for precision Timing in your hardware (and its operating software) are, of course, yours to determine... however, the ESPressio Timing library exists to facilitate your precision Timing needs.
