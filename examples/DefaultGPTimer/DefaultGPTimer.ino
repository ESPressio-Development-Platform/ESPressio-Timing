#include <Arduino.h>
#include <ESPressio_Timing.hpp>

using namespace ESPressio::Timing;

// Install System providers before constructing clock services in the composition application.
// The shared source uses its generic counter provider or the System monotonic fallback.
StopwatchClock<> stopwatch(true);

void setup() {
    Serial.begin(115200);

    HighResolutionTimeSource* source =
        HighResolutionTimeSource::GetInstance();

    Serial.print("Default source: ");
    Serial.println(
        source->GetIsUsingHighResolutionCounter()
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
