#include <ESPressio_ClockSynchronization.hpp>
#include "ClockTestEvidence.hpp"
#include <cassert>
using namespace ESPressio::Timing;
int main() {
    ClockSynchronizationProfile profile; profile.MaximumAcceptedRoundTripDelayNanoseconds=1000000;
    auto o=ClockTest::Observation(1000000,450,100,1,5);
    auto result=ValidateClockSynchronizationObservation(o,profile);
    assert(result.Accepted() && result.MeasuredOffsetNanoseconds==450 && result.RoundTripDelayNanoseconds==100);
    assert(result.Uncertainty.IsKnown && result.Uncertainty.Nanoseconds==65); // 5 + ceil(100/2) + ceil(20/2)
    auto bad=o; bad.T4.SystemTimeNanoseconds=o.T1.SystemTimeNanoseconds-1;
    assert(ValidateClockSynchronizationObservation(bad,profile).Rejection==ClockObservationRejection::InvalidTimestampOrder);
    bad=o; bad.T3.SystemTimeNanoseconds=bad.T2.SystemTimeNanoseconds+200;
    assert(ValidateClockSynchronizationObservation(bad,profile).Rejection==ClockObservationRejection::RemoteProcessingExceedsLocalElapsed);
    bad=o; bad.T4.SystemTimeNanoseconds+=1000000;
    assert(ValidateClockSynchronizationObservation(bad,profile).Rejection==ClockObservationRejection::RoundTripDelayExceeded);
    bad=o; bad.ReferenceIdentity=0;
    assert(ValidateClockSynchronizationObservation(bad,profile).Rejection==ClockObservationRejection::InvalidReference);
    bad=o; bad.T1.Quality=ClockCaptureQuality::Invalid;
    assert(ValidateClockSynchronizationObservation(bad,profile).Rejection==ClockObservationRejection::InvalidCaptureQuality);
    bad=o; bad.T1.Quality=ClockCaptureQuality::SoftwareUnbounded;
    result=ValidateClockSynchronizationObservation(bad,profile); assert(result.Accepted() && !result.Uncertainty.IsKnown);
    bad=o; bad.T1.Uncertainty.Nanoseconds=profile.MaximumCaptureUncertaintyNanoseconds+1;
    assert(ValidateClockSynchronizationObservation(bad,profile).Rejection==ClockObservationRejection::CaptureUncertaintyExceeded);
    bad=o; bad.ReferenceUncertainty=ClockUncertainty::Known(UINT64_MAX);
    assert(ValidateClockSynchronizationObservation(bad,profile).Rejection==ClockObservationRejection::NumericOverflow);
    o.HasCalibratedAsymmetryBound=true; o.CalibratedAsymmetryBoundNanoseconds=7;
    assert(ValidateClockSynchronizationObservation(o,profile).Uncertainty.Nanoseconds==22);
    assert(ClockMath::Average(1,-2)==0 && ClockMath::Average(-1,2)==0);
    assert(ClockMath::Average(INT64_MAX,INT64_MAX)==INT64_MAX && ClockMath::Average(INT64_MIN,INT64_MIN)==INT64_MIN);
    profile.MaximumAcceptedRoundTripDelayNanoseconds=0; assert(!profile.IsValid(8));
}
