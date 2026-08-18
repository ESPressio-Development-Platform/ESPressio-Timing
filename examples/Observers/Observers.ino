#include <ESPressio_Timing.hpp>

using namespace ESPressio;

class SystemClockObserver final :
    public Timing::ISystemClockObserver<
        Timing::ClockTick
    > {
public:
    void OnSystemClockSynchronized(
        Timing::ClockTick before,
        Timing::ClockTick after,
        int64_t immediateDifference,
        const Timing::ClockSynchronizationResult<
            Timing::ClockTick
        >& result,
        const Timing::ClockSynchronizationStatus<
            Timing::ClockTick
        >& status
    ) override {
        Serial.printf(
            "sync before=%llu after=%llu immediate=%lld measured=%lld filtered=%lld state=%u\n",
            static_cast<unsigned long long>(before),
            static_cast<unsigned long long>(after),
            static_cast<long long>(immediateDifference),
            static_cast<long long>(result.MeasuredOffsetNanoseconds),
            static_cast<long long>(result.FilteredOffsetNanoseconds),
            static_cast<unsigned int>(status.State)
        );
    }

    void OnSystemClockSynchronizationSampleRejected(
        const Timing::ClockSynchronizationResult<
            Timing::ClockTick
        >& result,
        const Timing::ClockSynchronizationStatus<
            Timing::ClockTick
        >&
    ) override {
        Serial.printf(
            "sync sample rejected: RTT=%llu ns\n",
            static_cast<unsigned long long>(
                result.RoundTripDelayNanoseconds
            )
        );
    }
};

SystemClockObserver clockObserver;
Observable::ObserverHandlePtr clockObserverHandle;

void setup() {
    Serial.begin(115200);

    clockObserverHandle =
        Timing::SystemClock<>::GetInstance().
            RegisterObserver(
                &clockObserver
            );
}

void loop() {
    /*
     * A transport such as ESPressio ESP-Now can submit synchronization samples
     * to SystemClock<>. This Observer will receive the resulting callbacks.
     */
    delay(1000);
}
