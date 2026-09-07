#include <cassert>
#include <cmath>
#include <cstdint>

#include <ESPressio_Timing.hpp>

using namespace ESPressio;
using namespace ESPressio::Timing;


static void TestSampleCalculation() {
    ClockSynchronizationConfig config;
    config.OffsetFilterWeight = 1.0;

    ClockDiscipline<uint64_t> discipline(config);
    ClockSynchronizationSample<uint64_t> sample;
    sample.LocalRequestTransmitTime = 1000;
    sample.RemoteRequestReceiveTime = 1500;
    sample.RemoteResponseTransmitTime = 1600;
    sample.LocalResponseReceiveTime = 1200;

    const auto result = discipline.SubmitSample(sample);
    assert(result.Accepted);
    assert(result.RoundTripDelayNanoseconds == 100);
    assert(result.MeasuredOffsetNanoseconds == 450);
    assert(result.FilteredOffsetNanoseconds == 450);
}


static void TestMalformedSampleRejection() {
    ClockDiscipline<uint64_t> discipline;
    ClockSynchronizationSample<uint64_t> sample;
    sample.LocalRequestTransmitTime = 1000;
    sample.LocalResponseReceiveTime = 1100;
    sample.RemoteRequestReceiveTime = 2000;
    sample.RemoteResponseTransmitTime = 2200;
    const auto result = discipline.SubmitSample(sample);
    assert(!result.Accepted);
    assert(result.RejectedSampleCount == 1);
}


static void TestPhaseSlew() {
    ClockSynchronizationConfig config;
    config.MaximumSlewRatePpm = 100000;
    config.OffsetFilterWeight = 1.0;

    ClockDiscipline<uint64_t> discipline(config);
    ClockSynchronizationSample<uint64_t> sample;
    sample.LocalRequestTransmitTime = 1000;
    sample.RemoteRequestReceiveTime = 2000;
    sample.RemoteResponseTransmitTime = 2000;
    sample.LocalResponseReceiveTime = 1000;

    const auto result = discipline.SubmitSample(sample);
    assert(result.Accepted);
    assert(result.MeasuredOffsetNanoseconds == 1000);

    discipline.Advance(1000);
    discipline.Advance(11000);
    assert(discipline.GetAppliedCorrectionNanoseconds() == 1000);
    assert(discipline.GetPendingPhaseCorrectionNanoseconds() == 0);
}


static void TestStep() {
    ClockDiscipline<uint64_t> discipline;
    discipline.ApplyStep(-250);
    assert(discipline.GetAppliedCorrectionNanoseconds() == -250);
    assert(discipline.GetPendingPhaseCorrectionNanoseconds() == 0);
}


static void TestDriftLearning() {
    ClockSynchronizationConfig config;
    config.OffsetFilterWeight = 1.0;
    config.DriftFilterWeight = 1.0;
    config.DriftLearningPhaseThresholdNanoseconds = 1000000ULL;
    config.MinimumDriftLearningIntervalNanoseconds = 1000000000ULL;

    ClockDiscipline<uint64_t> discipline(config);
    ClockSynchronizationSample<uint64_t> first;
    first.LocalRequestTransmitTime = 1000000ULL;
    first.RemoteRequestReceiveTime = 1000000ULL;
    first.RemoteResponseTransmitTime = 1000000ULL;
    first.LocalResponseReceiveTime = 1000000ULL;
    auto firstResult = discipline.SubmitSample(first);
    assert(firstResult.Accepted);

    ClockSynchronizationSample<uint64_t> second;
    second.LocalRequestTransmitTime = 1001000000ULL;
    second.RemoteRequestReceiveTime = 1001020000ULL;
    second.RemoteResponseTransmitTime = 1001020000ULL;
    second.LocalResponseReceiveTime = 1001000000ULL;
    auto secondResult = discipline.SubmitSample(second);
    assert(secondResult.Accepted);
    assert(std::fabs(secondResult.EstimatedDriftPpm - 20.0) < 0.001);
}


static ClockSynchronizationSample<uint64_t> OffsetAndDelaySample(
    uint64_t localTime,
    int64_t offset,
    uint64_t delay
) {
    ClockSynchronizationSample<uint64_t> sample;
    sample.LocalRequestTransmitTime = localTime;
    sample.RemoteRequestReceiveTime = static_cast<uint64_t>(
        static_cast<int64_t>(localTime) + offset + static_cast<int64_t>(delay / 2U));
    sample.RemoteResponseTransmitTime = sample.RemoteRequestReceiveTime;
    sample.LocalResponseReceiveTime = localTime + delay;
    return sample;
}


static void TestMinimumDelayClockFilterRejectsQueueExcursion() {
    ClockSynchronizationConfig config;
    config.OffsetFilterWeight = 1.0;
    config.ClockFilterWindowSamples = 4;

    ClockDiscipline<uint64_t> discipline(config);
    auto baseline = discipline.SubmitSample(
        OffsetAndDelaySample(1000000000ULL, 200000, 2000000ULL));
    assert(baseline.Accepted);
    assert(baseline.FilteredOffsetNanoseconds == 200000);

    auto excursion = discipline.SubmitSample(
        OffsetAndDelaySample(2000000000ULL, 5200000, 16000000ULL));
    assert(excursion.Accepted);
    assert(excursion.MeasuredOffsetNanoseconds == 5200000);
    assert(excursion.FilteredOffsetNanoseconds == 200000);
}


static void TestClockFilterCompensatesAppliedCorrection() {
    ClockSynchronizationConfig config;
    config.MaximumSlewRatePpm = 100000;
    config.OffsetFilterWeight = 1.0;
    config.ClockFilterWindowSamples = 4;

    ClockDiscipline<uint64_t> discipline(config);
    auto first = discipline.SubmitSample(
        OffsetAndDelaySample(1000000ULL, 1000, 100));
    assert(first.Accepted);
    discipline.Advance(1000000ULL);
    discipline.Advance(1010000ULL);
    assert(discipline.GetAppliedCorrectionNanoseconds() == 1000);

    auto queued = discipline.SubmitSample(
        OffsetAndDelaySample(2000000ULL, 4000, 10000));
    assert(queued.Accepted);
    assert(queued.MeasuredOffsetNanoseconds == 4000);
    assert(queued.FilteredOffsetNanoseconds == 0);
}


static void TestClockFilterReconfigurationClearsRetainedWindow() {
    ClockSynchronizationConfig config;
    config.OffsetFilterWeight = 1.0;
    config.ClockFilterWindowSamples = 4;
    ClockDiscipline<uint64_t> discipline(config);
    assert(discipline.SubmitSample(
        OffsetAndDelaySample(1000000ULL, 200000, 100)).Accepted);

    config.ClockFilterWindowSamples = 2;
    discipline.Configure(config);
    const auto afterReconfigure = discipline.SubmitSample(
        OffsetAndDelaySample(2000000ULL, 700000, 10000));
    assert(afterReconfigure.Accepted);
    assert(afterReconfigure.FilteredOffsetNanoseconds == 700000);
}


static void TestHardStepInvalidatesPreStepClockFilterHistory() {
    ClockSynchronizationConfig config;
    config.OffsetFilterWeight = 1.0;
    config.ClockFilterWindowSamples = 8;
    config.SynchronizationToleranceNanoseconds = 500000ULL;
    config.MinimumSamplesForSynchronizedState = 2U;

    ClockDiscipline<uint64_t> discipline(config);

    // The lowest-delay acquisition sample causes a large bootstrap step. Before the fix, this observation remained in
    // the minimum-delay window and could continue producing a synthetic zero residual after the clock had stepped.
    const auto acquisition = discipline.SubmitSample(
        OffsetAndDelaySample(1000000000ULL, 10000000, 100000ULL));
    assert(acquisition.Accepted);
    assert(acquisition.FilteredOffsetNanoseconds == 10000000);
    discipline.ApplyStep(acquisition.FilteredOffsetNanoseconds);

    auto afterStepStatus = discipline.GetStatus(1010100000ULL);
    assert(afterStepStatus.State == ClockSynchronizationState::Acquiring);
    assert(afterStepStatus.FilteredOffsetNanoseconds == 0);
    assert(afterStepStatus.PendingPhaseCorrectionNanoseconds == 0);

    // A fresh, slightly slower post-step exchange sees a genuine 3 ms residual. It MUST become the first filter
    // observation in the new coordinate system rather than losing to the old 100 us pre-step sample.
    const auto residual = discipline.SubmitSample(
        OffsetAndDelaySample(2000000000ULL, 3000000, 800000ULL));
    assert(residual.Accepted);
    assert(residual.MeasuredOffsetNanoseconds == 3000000);
    assert(residual.FilteredOffsetNanoseconds == 3000000);
    assert(discipline.GetPendingPhaseCorrectionNanoseconds() == 3000000);

    const auto residualStatus = discipline.GetStatus(2000800000ULL);
    assert(residualStatus.AcceptedSampleCount == 2U); // lifetime diagnostics remain cumulative
    assert(residualStatus.State == ClockSynchronizationState::Acquiring);
}


int main() {
    TestSampleCalculation();
    TestMalformedSampleRejection();
    TestPhaseSlew();
    TestStep();
    TestDriftLearning();
    TestMinimumDelayClockFilterRejectsQueueExcursion();
    TestClockFilterCompensatesAppliedCorrection();
    TestClockFilterReconfigurationClearsRetainedWindow();
    TestHardStepInvalidatesPreStepClockFilterHistory();

    return 0;
}
