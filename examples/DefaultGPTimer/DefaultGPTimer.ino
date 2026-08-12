#include <Arduino.h>
#include <ESPressio_Timing.hpp>

using namespace ESPressio::Timing;

// On a compatible ESP32 this uses the shared default GPTimer automatically.
// It falls back to esp_timer if the hardware timer cannot be initialized.
StopwatchClock stopwatch(true);

void setup() {
    Serial.begin(115200);

    HighResolutionTimeSource* source =
        HighResolutionTimeSource::GetInstance();

    Serial.print("Default source: ");
    Serial.println(
        source->GetIsUsingGPTimer()
            ? "GPTimer"
            : "framework monotonic timer"
    );

    Serial.print("Resolution: ");
    Serial.println(stopwatch.GetResolution().AsString());
}

void loop() {
    Serial.print("Elapsed: ");
    Serial.println(stopwatch.GetTime().AsString());
    delay(1000);
}
