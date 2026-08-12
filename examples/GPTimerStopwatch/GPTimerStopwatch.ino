#include <Arduino.h>
#include <ESPressio_Timing.hpp>

using namespace ESPressio::Timing;

#if ESPRESSIO_TIMING_HAS_GPTIMER

GPTimerClock stopwatch(
    true,
    ESPRESSIO_TIMING_GPTIMER_DEFAULT_RESOLUTION_HZ
);

#endif

void setup() {
    Serial.begin(115200);

    #if ESPRESSIO_TIMING_HAS_GPTIMER
        if (!stopwatch.GetIsAvailable()) {
            Serial.print("GPTimer initialization failed: ");
            Serial.println(stopwatch.GetInitializationResult());
        }
    #else
        Serial.println(
            "GPTimer requires ESP32 with the ESP-IDF 5.x GPTimer driver"
        );
    #endif
}

void loop() {
    #if ESPRESSIO_TIMING_HAS_GPTIMER
        if (stopwatch.GetIsAvailable()) {
            const ClockTime elapsed = stopwatch.GetTime();
            Serial.print("Elapsed: ");
            Serial.println(elapsed.AsString());
        }
    #endif

    delay(1000);
}
