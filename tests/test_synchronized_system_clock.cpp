#include <cassert>
#include <cstdint>

#include <ESPressio_Timing.hpp>
#include "SerializableTime.hpp"

using namespace ESPressio;
using namespace ESPressio::Timing;


class SynchronizationFakeSource :
    public ITimeSource {

    public:
        uint64_t ticks = 0;

        uint64_t GetTicks() const override {
            return ticks;
        }

        uint64_t GetTicksPerSecond() const override {
            return 1000000000ULL;
        }
};


int main() {
    SynchronizationFakeSource source;

    using Clock =
        SystemClock<
            DefaultClockTime,
            NoLockPolicy
        >;

    auto& clock =
        Clock::GetInstance(
            &source
        );

    clock.ResetSynchronization();

    ClockSynchronizationConfig config;
    config.OffsetFilterWeight = 1.0;
    config.MinimumSamplesForSynchronizedState = 1;

    clock.ConfigureSynchronization(
        config
    );

    source.ticks = 1000000;

    const uint64_t t1 =
        clock.
            GetSynchronizationTimestampNanoseconds();

    /*
     * Perfect zero-delay exchange with the remote clock exactly 5 ms ahead.
     */
    ClockSynchronizationSample<uint64_t> sample;
    sample.LocalRequestTransmitTime = t1;
    sample.RemoteRequestReceiveTime =
        t1 + 5000000ULL;
    sample.RemoteResponseTransmitTime =
        t1 + 5000000ULL;
    sample.LocalResponseReceiveTime = t1;

    const auto result =
        clock.SubmitSynchronizationSample(
            sample,
            ClockSynchronizationAdjustmentMode::
                StepIfUnsynchronized
        );

    assert(result.Accepted);
    assert(
        result.MeasuredOffsetNanoseconds ==
        5000000
    );

    /*
     * The explicit startup step is visible immediately.
     */
    const uint64_t corrected =
        clock.
            GetSynchronizationTimestampNanoseconds();

    assert(
        corrected ==
        t1 + 5000000ULL
    );

    const auto status =
        clock.GetSynchronizationStatus();

    assert(status.HasAcceptedSample);
    assert(
        status.State ==
        ClockSynchronizationState::
            Synchronized
    );

    /*
     * The synchronization target interface is representation-independent.
     */
    IClockSynchronizationTarget<
        ClockTick
    >& target =
        clock;

    assert(
        target.
            GetSynchronizationTimestampNanoseconds() ==
        corrected
    );

    /*
     * A second typed facade uses the same synchronized SystemClockCore.
     */
    using SerializableClockTime =
        Units::
            SerializableNanoSeconds<
                uint64_t
            >;

    using SerializableClock =
        SystemClock<
            SerializableClockTime,
            NoLockPolicy
        >;

    auto& serializableClock =
        SerializableClock::
            GetInstance(
                &source
            );

    assert(
        serializableClock.
            GetSynchronizationTimestampNanoseconds() ==
        corrected
    );

    SerializableClockTime typedTime =
        serializableClock.GetTime();

    assert(
        TimeTraits<
            SerializableClockTime
        >::template ToNanoseconds<
            uint64_t
        >(typedTime) ==
        corrected
    );

    return 0;
}
