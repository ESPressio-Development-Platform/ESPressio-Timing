#include <Arduino.h>
#include <ESPressio_Timing.hpp>

using namespace ESPressio::Timing;

StopwatchClock stopwatch(true);

void setup() {
    Serial.begin(115200);
}

void loop() {
    Serial.print("Elapsed time: ");
    Serial.println(stopwatch.GetTime().AsString());

    delay(1000);
}
