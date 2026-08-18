#include <Arduino.h>

#include <ESPressio_Timing.hpp>

using namespace ESPressio;
using namespace ESPressio::Timing;

using Clock =
    SystemClock<>;

Clock& systemClock =
    Clock::GetInstance();


void SubmitCompletedSynchronizationExchange(
    uint64_t localRequestTransmitTime,
    uint64_t remoteRequestReceiveTime,
    uint64_t remoteResponseTransmitTime,
    uint64_t localResponseReceiveTime,
    bool permitStartupStep
) {
    ClockSynchronizationSample<
        ClockTick
    > sample;

    sample.LocalRequestTransmitTime =
        localRequestTransmitTime;

    sample.RemoteRequestReceiveTime =
        remoteRequestReceiveTime;

    sample.RemoteResponseTransmitTime =
        remoteResponseTransmitTime;

    sample.LocalResponseReceiveTime =
        localResponseReceiveTime;

    const auto result =
        systemClock.
            SubmitSynchronizationSample(
                sample,
                permitStartupStep
                    ? ClockSynchronizationAdjustmentMode::
                        StepIfUnsynchronized
                    : ClockSynchronizationAdjustmentMode::
                        SlewOnly
            );

    if (!result.Accepted) {
        Serial.println(
            "Synchronization sample rejected"
        );

        return;
    }

    Serial.printf(
        "offset=%lld ns delay=%llu ns drift=%.3f ppm\n",
        static_cast<long long>(
            result.
                MeasuredOffsetNanoseconds
        ),
        static_cast<unsigned long long>(
            result.
                RoundTripDelayNanoseconds
        ),
        result.EstimatedDriftPpm
    );
}


void setup() {
    Serial.begin(115200);

    ClockSynchronizationConfig
        configuration;

    configuration.
        MaximumRoundTripDelayNanoseconds =
            10000000ULL; // 10 ms

    configuration.
        MaximumSlewRatePpm =
            500;

    configuration.
        SynchronizationToleranceNanoseconds =
            500000ULL; // 0.5 ms

    systemClock.
        ConfigureSynchronization(
            configuration
        );

    /*
     * A transport such as ESP-NOW performs this sequence:
     *
     *   T1 = local clock capture immediately before request transmit
     *
     *   T2 = remote clock capture when the request is received
     *
     *   T3 = remote clock capture immediately before the response is sent
     *
     *   T4 = local clock capture when the response is received
     *
     * Capture a timestamp with:
     *
     *   systemClock.GetSynchronizationTimestampNanoseconds()
     *
     * Then pass the four timestamps to
     * SubmitCompletedSynchronizationExchange().
     *
     * ESPressio Timing deliberately contains no packet, MAC-address, Wi-Fi,
     * ESP-NOW, or peer-selection concepts.
     */
}


void loop() {
    const auto status =
        systemClock.
            GetSynchronizationStatus();

    switch (status.State) {
        case ClockSynchronizationState::
            Unsynchronized:
            break;

        case ClockSynchronizationState::
            Acquiring:
            break;

        case ClockSynchronizationState::
            Synchronized:
            break;
    }

    systemClock.Update();

    delay(1000);
}
