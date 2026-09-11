#include <ESPressio_ClockDiscipline.hpp>
#include "ClockTestEvidence.hpp"
#include <cassert>
using namespace ESPressio::Timing;
static std::uint64_t Acquire(ClockDiscipline<8>& discipline,std::int64_t phase=0,std::uint64_t capture=100,bool unknown=false) {
    discipline.SelectReference(1,0);
    std::uint64_t last=0;
    for (unsigned i=0;i<5;++i) {
        const auto now=1000000000ull+i*250000000ull;
        const auto model=discipline.Model();
        auto o=ClockTest::Observation(now,phase,10000,1,capture,&model);
        if (unknown) o.T1.Quality=ClockCaptureQuality::SoftwareUnbounded;
        assert(discipline.Submit(o,now+10000).Accepted); last=now+10000;
    }
    return last;
}
int main() {
    ClockDiscipline<8> unknown;
    auto now=Acquire(unknown,0,100,true);
    assert(unknown.GetStatus(now).Reliability==TimeReliability::Acquiring && !unknown.GetStatus(now).CurrentUncertainty.IsKnown);
    ClockDiscipline<8> highUncertainty;
    now=Acquire(highUncertainty,0,200000);
    assert(highUncertainty.GetStatus(now).CurrentUncertainty.Nanoseconds>500000);
    assert(highUncertainty.GetStatus(now).Reliability==TimeReliability::Acquiring);
    ClockDiscipline<8> immature;
    ClockSynchronizationProfile wide; wide.MinimumRegressionObservationSpanNanoseconds=2000000000;
    assert(immature.Configure(wide,0)==ClockConfigurationStatus::Success);
    now=Acquire(immature);
    assert(immature.GetStatus(now).InlierSamples>=4 && immature.GetStatus(now).Reliability==TimeReliability::Acquiring);
    ClockDiscipline<8> healthy; now=Acquire(healthy);
    const auto before=healthy.GetStatus(now); assert(before.Reliability==TimeReliability::Synchronized);
    const auto model=healthy.Model();
    const auto wanted=1000000-model.AnchorUncertainty.Nanoseconds;
    const auto expires=((wanted-1)*1000000000ull)/model.ResidualErrorPartsPerBillion+1;
    assert(IsQualifiedTimeReliability(healthy.GetStatus(now+expires-1).Reliability));
    assert(healthy.GetStatus(now+expires).CurrentUncertainty.Nanoseconds>=1000000);
    assert(healthy.GetStatus(now+expires).Reliability==TimeReliability::Acquiring);
    // Outlying evidence does not replace a healthy retained model or step the clock.
    auto outlier=ClockTest::Observation(now+250000000,5000000,10000,1,100,&model);
    const auto result=healthy.Submit(outlier,now+250010000);
    assert(!result.Accepted && result.Rejection==ClockObservationRejection::Outlier);
    assert(healthy.Model().AnchorMonotonic==model.AnchorMonotonic && healthy.GetStatus(now).AcceptedSamples==before.AcceptedSamples);
    // Slew settlement cannot tighten the bound without a new accepted observation.
    ClockDiscipline<8> largePhase; now=Acquire(largePhase,10000000);
    const auto initial=largePhase.GetStatus(now);
    assert(initial.Reliability==TimeReliability::Acquiring);
    const auto later=now+30000000000ull;
    assert(largePhase.Model().PendingPhase(later)==0);
    assert(largePhase.GetStatus(later).CurrentUncertainty.Nanoseconds>=initial.CurrentUncertainty.Nanoseconds);
    assert(largePhase.GetStatus(later).Reliability==TimeReliability::Acquiring);
    const auto settled=largePhase.Model();
    assert(largePhase.Submit(ClockTest::Observation(later,10000000,10000,1,100,&settled),later+10000).Accepted);
    assert(largePhase.GetStatus(later+10000).Reliability==TimeReliability::Synchronized);
    // A stronger configured physical drift bound forces an earlier evidence deadline.
    ClockDiscipline<8> fast,slow;
    ClockSynchronizationProfile profile; profile.ResidualFrequencyErrorBoundPpm=200;
    assert(fast.Configure(profile,0)==ClockConfigurationStatus::Success);
    profile.ResidualFrequencyErrorBoundPpm=20;
    assert(slow.Configure(profile,0)==ClockConfigurationStatus::Success);
    now=Acquire(fast); Acquire(slow);
    assert(fast.GetStatus(now).NextRequiredSynchronizationMonotonic<slow.GetStatus(now).NextRequiredSynchronizationMonotonic);
    fast.RecordSchedulerDeadlineMiss(); assert(fast.GetStatus(now).SchedulerDeadlineMisses==1);
}
