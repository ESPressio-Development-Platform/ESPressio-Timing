#include <ESPressio_Timing.hpp>
#if !defined(ESPRESSIO_TIMING_REAL_DEPENDENCIES)
#include "SerializableTime.hpp"
#endif
#include "ClockTestEvidence.hpp"
#include <cassert>
#include <cstdlib>
#include <new>
using namespace ESPressio;
using namespace ESPressio::Timing;
static bool denyHeap=false;
void* operator new(std::size_t n) { if (denyHeap) std::abort(); if (auto* p=std::malloc(n ? n : 1)) return p; throw std::bad_alloc(); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p,std::size_t) noexcept { std::free(p); }
class Source final : public ITimeSource {
public:
    std::uint64_t Now=0;
    std::uint64_t GetTicks() const override { return Now; }
    std::uint64_t GetTicksPerSecond() const override { return 1000000000; }
};
int main() {
    Source source;
    using Clock=SystemClock<DefaultClockTime,NoLockPolicy>;
    auto& clock=Clock::GetInstance(&source);
#if defined(ESPRESSIO_TIMING_REAL_DEPENDENCIES)
    using Other=SystemClock<Units::MicroSeconds<std::uint64_t>,NoLockPolicy>;
#else
    using Other=SystemClock<Units::SerializableNanoSeconds<std::uint64_t>,NoLockPolicy>;
#endif
    auto& other=Other::GetInstance(&source);
    assert(clock.TrySetTime(DefaultClockTime(0,Units::Nano))==ClockConfigurationStatus::Success);
    ClockSynchronizationProfile profile;
    assert(clock.ConfigureSynchronization(profile)==ClockConfigurationStatus::Success);
    assert(clock.SelectSynchronizationReference(1)==ClockConfigurationStatus::Success);
    clock.SealContinuity(); assert(other.IsContinuitySealed());
    assert(clock.TrySetTime(DefaultClockTime(1,Units::Base))==ClockConfigurationStatus::ContinuitySealed);
    IClockSynchronizationTarget& target=clock;
    for (unsigned i=0;i<5;++i) {
        source.Now=1000000000ull+i*250000000ull;
        auto model=clock.GetClockModelSnapshot();
        auto observation=ClockTest::Observation(source.Now,10000,10000,1,100,&model);
        source.Now+=10000;
        const auto before=clock.CaptureQualifiedTime().Nanoseconds;
        assert(target.SubmitSynchronizationObservation(observation).Accepted);
        assert(clock.CaptureQualifiedTime().Nanoseconds==before);
    }
    assert(target.GetSynchronizationStatus().Reliability==TimeReliability::Synchronized);
    denyHeap=true;
    const auto captured=target.CaptureSynchronizationTimestamp(ClockCaptureQuality::SoftwareBounded,ClockUncertainty::Known(100));
    assert(captured.SystemTimeNanoseconds==other.CaptureQualifiedTime().Nanoseconds && captured.MonotonicTimeNanoseconds==source.Now);
    for (unsigned i=0;i<1000;++i) {
        const auto previous=clock.CaptureQualifiedTime().Nanoseconds;
        ++source.Now; assert(clock.CaptureQualifiedTime().Nanoseconds>=previous);
        (void)target.GetSynchronizationStatus(); (void)other.GetTime();
    }
    source.Now+=30000000000ull;
    assert(clock.CaptureQualifiedTime().Reliability==TimeReliability::Acquiring);
    denyHeap=false;
    auto before=clock.CaptureQualifiedTime().Nanoseconds;
    clock.SelectSynchronizationReference(2);
    assert(clock.CaptureQualifiedTime().Nanoseconds==before && clock.GetSynchronizationStatus().RetainedSamples==0);
}
