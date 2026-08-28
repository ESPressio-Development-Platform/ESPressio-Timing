# High Resolution Sources

Timing's reusable high-resolution source logic consumes ESPressio System clock capabilities rather than selecting target timer APIs itself.

A normal high-resolution source can use a dedicated `System::Clock::IHighResolutionCounter` when available and fall back to the installed System monotonic clock where the Timing abstraction permits it.

## Generic terminology

New code should use `GetIsUsingHighResolutionCounter()` and generic high-resolution-counter terminology.

The historical `GPTimerTimeSource` and `GetIsUsingGPTimer()` names remain compatibility surfaces over the generic System high-resolution-counter capability; they do not give Timing ownership of ESP-IDF GPTimer.

## Platform setup

The target platform package installs the actual monotonic clock/high-resolution counter provider. Timing never exposes the native timer handle or native SDK error type.

## Consumer implication

Code using Timing should ask for the timing semantics it needs, not for a specific ESP32 peripheral. This allows the same Timing code to run on another platform provider.