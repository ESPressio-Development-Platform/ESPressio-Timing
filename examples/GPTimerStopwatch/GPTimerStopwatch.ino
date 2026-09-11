#include <Arduino.h>
#include <ESPressio_Timing.hpp>

using namespace ESPressio::Timing;

#if ESPRESSIO_TIMING_HAS_GPTIMER

GPTimerClock<> stopwatch(
    true,
    ESPRESSIO_TIMING_GPTIMER_DEFAULT_RESOLUTION_HZ
);

#endif

void setup() {
    Serial.begin(115200);

    #if ESPRESSIO_TIMING_HAS_GPTIMER
        if (!stopwatch.GetIsAvailable()) {
            Serial.print("GPTimer initialization failed: ");
            Serial.println(static_cast<unsigned>(stopwatch.GetInitializationResult().Status));
        }
    #else
        Serial.println(
            "The selected build disables the generic counter-backed clock surface"
        );
    #endif
}

void loop() {
    #if ESPRESSIO_TIMING_HAS_GPTIMER
        if (stopwatch.GetIsAvailable()) {
            const DefaultClockTime elapsed = stopwatch.GetTime();
            Serial.print("Elapsed: ");
            Serial.println(elapsed.AsString());
        }
    #endif

    delay(1000);
}
