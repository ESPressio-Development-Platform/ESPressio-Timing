#include <ESPressio_ClockDiscipline.hpp>
#include "ClockTestEvidence.hpp"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <new>
using namespace ESPressio::Timing;
static bool denyHeap=false;
void* operator new(std::size_t n) { if (denyHeap) std::abort(); if (auto* p=std::malloc(n ? n : 1)) return p; throw std::bad_alloc(); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p,std::size_t) noexcept { std::free(p); }
int main() {
    denyHeap=true;
    ClockSynchronizationProfile profile;
    for (int ppm:std::array<int,3>{0,20,-20}) {
        ClockRegression<8> regression;
        constexpr std::uint64_t epoch=UINT64_MAX-20000000000ull;
        for (unsigned i=0;i<12;++i) {
            const std::uint64_t elapsed=i*1000000000ull;
            const auto y=1000+static_cast<std::int64_t>(i)*ppm*1000;
            regression.Add({epoch+elapsed,y,y,ClockUncertainty::Known(100+i*10),true});
            const auto fit=regression.Calculate(profile); assert(fit.Valid);
            if (i>0) assert(std::fabs(fit.Slope*1000000-ppm)<0.000001);
            assert(std::fabs(fit.Intercept-y)<0.001);
        }
        assert(regression.Size()==8);
    }
    ClockDiscipline<8> discipline;
    assert(discipline.SelectReference(1,0)==ClockConfigurationStatus::Success);
    assert(discipline.TryRebase(0,0)==ClockConfigurationStatus::Success);
    discipline.SealContinuity();
    for (unsigned i=0;i<8;++i) {
        const auto now=1000000000ull+i*250000000ull;
        auto model=discipline.Model();
        auto observation=ClockTest::Observation(now,1000,10000,1,100,&model);
        const auto before=discipline.Model().Evaluate(now+10000);
        const auto result=discipline.Submit(observation,now+10000);
        assert(result.Accepted && discipline.Model().Evaluate(now+10000)==before);
        if (i<3) assert(discipline.GetStatus(now+10000).Reliability==TimeReliability::Acquiring);
    }
    const auto anchor=discipline.Model().AnchorMonotonic;
    auto status=discipline.GetStatus(anchor);
    assert(status.Reliability==TimeReliability::Synchronized && status.CurrentUncertainty.Nanoseconds<=500000);
    const auto deadline=status.NextRequiredSynchronizationMonotonic;
    assert(deadline>anchor && deadline-anchor!=1000000000ull);
    const auto uncertainty=status.CurrentUncertainty.Nanoseconds;
    assert(discipline.GetStatus(anchor+1000000).CurrentUncertainty.Nanoseconds>=uncertainty);
    discipline.SetActivity(true,false,anchor);
    assert(discipline.GetStatus(anchor).Reliability==TimeReliability::Holdover);
    assert(discipline.GetStatus(anchor+30000000000ull).Reliability==TimeReliability::Acquiring);
    discipline.SetActivity(false,false,anchor);
    assert(discipline.GetStatus(anchor+30000000000ull).Reliability==TimeReliability::Unqualified);
    const auto old=discipline.Model().Evaluate(anchor);
    assert(discipline.TryRebase(0,anchor)==ClockConfigurationStatus::ContinuitySealed && discipline.Model().Evaluate(anchor)==old);
    assert(discipline.SelectReference(2,anchor)==ClockConfigurationStatus::Success && discipline.Model().Evaluate(anchor)==old);
    assert(discipline.GetStatus(anchor).Reliability==TimeReliability::Acquiring && discipline.GetStatus(anchor).RetainedSamples==0);
    assert(discipline.Submit(ClockTest::Observation(anchor+1000000),anchor+1010000).Rejection==ClockObservationRejection::ReferenceMismatch);
    // Separate rounding of two negative rate terms must never cause a one-nanosecond backward step.
    ClockModelSnapshot model; model.AnchorTime=1000000; model.FrequencyPartsPerBillion=-2000000;
    model.SlewPartsPerBillion=500000; model.PendingPhaseNanoseconds=-1000;
    auto previous=model.Evaluate(0);
    for (std::uint64_t n=1;n<2200000;++n) { const auto next=model.Evaluate(n); assert(next>=previous); previous=next; }
    assert(model.PendingPhase(2200000)==0);
}
