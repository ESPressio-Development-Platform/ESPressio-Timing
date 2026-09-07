#include <cassert>
#include <cstdint>

#include <ESPressio_ClockSynchronization.hpp>

using namespace ESPressio::Timing;

int main() {
    constexpr std::uint64_t maximumRoundTripDelay = 1000000ULL;

    ClockSynchronizationSample<std::uint64_t> accepted{};
    accepted.LocalRequestTransmitTime = 1000000ULL;
    accepted.RemoteRequestReceiveTime = 1200000ULL;
    accepted.RemoteResponseTransmitTime = 1250000ULL;
    accepted.LocalResponseReceiveTime = 1600000ULL;
    auto validation = ValidateClockSynchronizationSample(accepted, maximumRoundTripDelay);
    assert(validation.Accepted());
    assert(validation.LocalElapsedNanoseconds == 600000ULL);
    assert(validation.RemoteProcessingElapsedNanoseconds == 50000ULL);
    assert(validation.RoundTripDelayNanoseconds == 550000ULL);

    auto invalidOrder = accepted;
    invalidOrder.LocalResponseReceiveTime = invalidOrder.LocalRequestTransmitTime - 1ULL;
    validation = ValidateClockSynchronizationSample(invalidOrder, maximumRoundTripDelay);
    assert(!validation.Accepted());
    assert(validation.RejectionReason ==
           ClockSynchronizationSampleRejectionReason::InvalidTimestampOrder);

    auto impossibleProcessing = accepted;
    impossibleProcessing.LocalResponseReceiveTime = 1200000ULL;
    impossibleProcessing.RemoteRequestReceiveTime = 1200000ULL;
    impossibleProcessing.RemoteResponseTransmitTime = 1500000ULL;
    validation = ValidateClockSynchronizationSample(impossibleProcessing, maximumRoundTripDelay);
    assert(!validation.Accepted());
    assert(validation.RejectionReason ==
           ClockSynchronizationSampleRejectionReason::RemoteProcessingExceedsLocalElapsed);
    assert(validation.LocalElapsedNanoseconds == 200000ULL);
    assert(validation.RemoteProcessingElapsedNanoseconds == 300000ULL);

    auto excessiveDelay = accepted;
    excessiveDelay.LocalResponseReceiveTime = 2600000ULL;
    excessiveDelay.RemoteRequestReceiveTime = 1200000ULL;
    excessiveDelay.RemoteResponseTransmitTime = 1250000ULL;
    validation = ValidateClockSynchronizationSample(excessiveDelay, maximumRoundTripDelay);
    assert(!validation.Accepted());
    assert(validation.RejectionReason ==
           ClockSynchronizationSampleRejectionReason::RoundTripDelayExceeded);
    assert(validation.RoundTripDelayNanoseconds == 1550000ULL);

    validation = ValidateClockSynchronizationSample(excessiveDelay, 0U);
    assert(validation.Accepted());
    assert(validation.RoundTripDelayNanoseconds == 1550000ULL);

    return 0;
}
