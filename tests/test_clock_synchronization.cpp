#include <cassert>
#include <cmath>
#include <cstdint>

#include <ESPressio_Timing.hpp>

using namespace ESPressio;
using namespace ESPressio::Timing;


static void TestSampleCalculation() {
    ClockSynchronizationConfig config;
    config.OffsetFilterWeight = 1.0;

    ClockDiscipline<uint64_t> discipline(
        config
    );

    ClockSynchronizationSample<uint64_t> sample;

    sample.LocalRequestTransmitTime = 1000;
    sample.RemoteRequestReceiveTime = 1500;
    sample.RemoteResponseTransmitTime = 1600;
    sample.LocalResponseReceiveTime = 1200;

    const auto result =
        discipline.SubmitSample(
            sample
        );

    assert(result.Accepted);

    /*
     * Delay = (T4-T1) - (T3-T2)
     *       = 200 - 100 = 100 ns
     *
     * Offset = ((T2-T1) + (T3-T4)) / 2
     *        = (500 + 400) / 2 = 450 ns
     */
    assert(
        result.RoundTripDelayNanoseconds ==
        100
    );

    assert(
        result.MeasuredOffsetNanoseconds ==
        450
    );

    assert(
        result.FilteredOffsetNanoseconds ==
        450
    );
}


static void TestMalformedSampleRejection() {
    ClockDiscipline<uint64_t> discipline;

    ClockSynchronizationSample<uint64_t> sample;

    sample.LocalRequestTransmitTime = 1000;
    sample.LocalResponseReceiveTime = 1100;

    sample.RemoteRequestReceiveTime = 2000;
    sample.RemoteResponseTransmitTime = 2200;

    /*
     * Remote processing time (200 ns) exceeds the entire local exchange
     * duration (100 ns), giving a physically impossible negative network
     * round-trip delay.
     */
    const auto result =
        discipline.SubmitSample(
            sample
        );

    assert(!result.Accepted);
    assert(
        result.RejectedSampleCount ==
        1
    );
}


static void TestPhaseSlew() {
    ClockSynchronizationConfig config;
    config.MaximumSlewRatePpm = 100000;
    config.OffsetFilterWeight = 1.0;

    ClockDiscipline<uint64_t> discipline(
        config
    );

    ClockSynchronizationSample<uint64_t> sample;

    sample.LocalRequestTransmitTime = 1000;
    sample.RemoteRequestReceiveTime = 2000;
    sample.RemoteResponseTransmitTime = 2000;
    sample.LocalResponseReceiveTime = 1000;

    const auto result =
        discipline.SubmitSample(
            sample
        );

    assert(result.Accepted);
    assert(
        result.MeasuredOffsetNanoseconds ==
        1000
    );

    discipline.Advance(1000);

    /*
     * At 100,000 ppm, 10,000 ns of raw progression allows 1,000 ns of
     * monotonic phase correction.
     */
    discipline.Advance(11000);

    assert(
        discipline.
            GetAppliedCorrectionNanoseconds() ==
        1000
    );

    assert(
        discipline.
            GetPendingPhaseCorrectionNanoseconds() ==
        0
    );
}


static void TestStep() {
    ClockDiscipline<uint64_t> discipline;

    discipline.ApplyStep(
        -250
    );

    assert(
        discipline.
            GetAppliedCorrectionNanoseconds() ==
        -250
    );

    assert(
        discipline.
            GetPendingPhaseCorrectionNanoseconds() ==
        0
    );
}


static void TestDriftLearning() {
    ClockSynchronizationConfig config;
    config.OffsetFilterWeight = 1.0;
    config.DriftFilterWeight = 1.0;
    config.DriftLearningPhaseThresholdNanoseconds = 1000000ULL;
    config.MinimumDriftLearningIntervalNanoseconds = 1000000000ULL;

    ClockDiscipline<uint64_t> discipline(
        config
    );

    ClockSynchronizationSample<uint64_t> first;
    first.LocalRequestTransmitTime = 1000000ULL;
    first.RemoteRequestReceiveTime = 1000000ULL;
    first.RemoteResponseTransmitTime = 1000000ULL;
    first.LocalResponseReceiveTime = 1000000ULL;

    auto firstResult =
        discipline.SubmitSample(
            first
        );

    assert(firstResult.Accepted);

    /*
     * The phase servo is already settled because the first offset is zero.
     * One second later the remote clock is 20 us farther ahead, representing
     * a +20 ppm relative rate error.
     */
    ClockSynchronizationSample<uint64_t> second;
    second.LocalRequestTransmitTime = 1001000000ULL;
    second.RemoteRequestReceiveTime = 1001020000ULL;
    second.RemoteResponseTransmitTime = 1001020000ULL;
    second.LocalResponseReceiveTime = 1001000000ULL;

    auto secondResult =
        discipline.SubmitSample(
            second
        );

    assert(secondResult.Accepted);

    assert(
        std::fabs(
            secondResult.EstimatedDriftPpm -
            20.0
        ) <
        0.001
    );
}


int main() {
    TestSampleCalculation();
    TestMalformedSampleRejection();
    TestPhaseSlew();
    TestStep();
    TestDriftLearning();

    return 0;
}
