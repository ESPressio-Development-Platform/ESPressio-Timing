#include <Arduino.h>

#include <ESPressio_Timing.hpp>

/*
 * Optional. Only projects that actually want a serializable public time
 * representation include the Serializable Unit header and add
 * ESPressio-Serializable to lib_deps.
 *
 * #include <ESPressio_Time_Serializable.hpp>
 */

using namespace ESPressio;
using namespace ESPressio::Timing;

void setup() {
    Serial.begin(115200);

    /*
     * Ordinary Timing: no Serializable dependency.
     */
    StopwatchClock<> ordinary(
        true
    );

    DefaultClockTime elapsed =
        ordinary.GetTime();

    Serial.println(
        elapsed.AsString()
    );


    /*
     * Serializable public representation:
     *
     * using SerializableClockTime =
     *     Units::SerializableNanoSeconds<uint64_t>;
     *
     * StopwatchClock<
     *     SerializableClockTime
     * > serializableClock(true);
     *
     * SerializableClockTime serializableElapsed =
     *     serializableClock.GetTime();
     *
     * The stopwatch algorithm and raw tick storage are identical.
     */
}

void loop() {
}
