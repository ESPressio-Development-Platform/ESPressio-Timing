#include <Arduino.h>
#include <ESPressio_TimingSystemClock.hpp>
using namespace ESPressio::Timing;
void SubmitCompletedSynchronizationExchange(const ClockSynchronizationObservation<>& observation) {
    const auto result=SystemClock<>::GetInstance().SubmitSynchronizationObservation(observation);
    Serial.printf("accepted=%u reason=%u offset=%lld ns RTT=%llu ns frequency=%.3f ppm\n",
        result.Accepted,static_cast<unsigned>(result.Rejection),static_cast<long long>(result.MeasuredOffsetNanoseconds),
        static_cast<unsigned long long>(result.RoundTripDelayNanoseconds),result.EstimatedFrequencyCorrectionPpm);
}
void setup() {
    Serial.begin(115200);
    auto& clock=SystemClock<>::GetInstance();
    ClockSynchronizationProfile profile;
    clock.ConfigureSynchronization(profile);
    clock.SelectSynchronizationReference(1); // replace with the orchestration-selected trusted lineage token
    clock.SealContinuity();
    // The transport captures T1/T2/T3/T4 with original monotonic coordinates,
    // reference reliability and conservative capture/reference bounds. Its
    // wake/deadline service uses GetSynchronizationStatus().NextRequiredSynchronizationMonotonic.
    // No fabricated exchange or fixed-period synchronization loop is provided.
}
void loop() {
    // Service general callbacks/diagnostics from an application-owned wake loop.
    // A real transport separately schedules the adaptive evidence deadline.
    SystemClock<>::GetInstance().Update();
    yield();
}
