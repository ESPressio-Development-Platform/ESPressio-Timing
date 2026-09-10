#include <cassert>
#include <cstdint>

#include <ESPressio_Timing.hpp>

using namespace ESPressio;
using namespace ESPressio::Timing;


class SharedFakeTimeSource final : public ITimeSource {
public:
    uint64_t Ticks = 0;

    uint64_t GetTicks() const override { return Ticks; }
    uint64_t GetTicksPerSecond() const override { return 1000000ULL; }
};

static uint64_t Nanoseconds(const DefaultClockTime& value) {
    // Host Timing tests intentionally use the lightweight ESPressio_Time test double rather than the complete Units
    // implementation. Preserve the semantic conversion exercised by this test without depending on Units::ToMagnitude.
    switch (value.orderOfMagnitude) {
        case Units::Nano: return value.value;
        case Units::Micro: return value.value * 1000ULL;
        case Units::Milli: return value.value * 1000000ULL;
        case Units::Base: return value.value * NanosecondsPerSecond;
    }
    assert(false && "unsupported host test time magnitude");
    return 0U;
}

int main() {
    SharedFakeTimeSource source;

    using TestSystemClock = SystemClock<DefaultClockTime, NoLockPolicy>;
    auto& systemClock = TestSystemClock::GetInstance(&source);
    MonotonicClock<> monotonicClock(&source);

    source.Ticks = 1000000ULL;
    const auto monotonicAtOneSecond = Nanoseconds(monotonicClock.GetTime());
    assert(monotonicAtOneSecond == 1000000000ULL);

    // Moving the distributed/settable SystemClock forward must not move the
    // independent monotonic scheduling timeline.
    systemClock.SetTime(DefaultClockTime(175, Units::Base));
    const auto monotonicAfterForwardStep = Nanoseconds(monotonicClock.GetTime());
    assert(monotonicAfterForwardStep == monotonicAtOneSecond);

    source.Ticks += 250ULL;
    const auto monotonicAfterElapsedTime = Nanoseconds(monotonicClock.GetTime());
    assert(monotonicAfterElapsedTime == monotonicAtOneSecond + 250000ULL);

    // A backward SystemClock step must likewise have no effect on elapsed time.
    systemClock.SetTime(DefaultClockTime(1, Units::Base));
    assert(Nanoseconds(monotonicClock.GetTime()) == monotonicAfterElapsedTime);

    source.Ticks += 750ULL;
    assert(Nanoseconds(monotonicClock.GetTime()) == monotonicAtOneSecond + 1000000ULL);

    // The process-wide default facade must resolve to the one shared Timing
    // high-resolution source rather than owning a per-clock/per-thread source.
    auto& sharedMonotonic = MonotonicClock<>::GetInstance();
    assert(sharedMonotonic.GetTimeSource() == HighResolutionTimeSource::GetInstance());

    return 0;
}
