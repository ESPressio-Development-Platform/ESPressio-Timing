#pragma once
#include <ESPressio_ClockSynchronization.hpp>
#include <ESPressio_ClockModelSnapshot.hpp>
namespace ClockTest {
inline ESPressio::Timing::ClockSynchronizationObservation<> Observation(
    std::uint64_t monotonic,std::int64_t offset=0,std::uint64_t delay=10000,
    std::uint64_t reference=1,std::uint64_t uncertainty=100,
    const ESPressio::Timing::ClockModelSnapshot* localModel=nullptr) {
    using namespace ESPressio::Timing;
    ClockSynchronizationObservation<> o;
    o.ReferenceIdentity=reference; o.ReferenceReliability=TimeReliability::Synchronized;
    o.ReferenceUncertainty=ClockUncertainty::Known(uncertainty);
    const auto middle=monotonic+delay/2;
    const auto remote=ClockMath::Apply(middle,offset);
    const auto t1=localModel ? localModel->Evaluate(monotonic) : monotonic;
    const auto t4=localModel ? localModel->Evaluate(monotonic+delay) : monotonic+delay;
    o.T1={t1,monotonic,ClockUncertainty::Known(uncertainty),ClockCaptureQuality::SoftwareBounded};
    o.T2={remote,middle,ClockUncertainty::Known(uncertainty),ClockCaptureQuality::SoftwareBounded};
    o.T3=o.T2;
    o.T4={t4,monotonic+delay,ClockUncertainty::Known(uncertainty),ClockCaptureQuality::SoftwareBounded};
    return o;
}
}
