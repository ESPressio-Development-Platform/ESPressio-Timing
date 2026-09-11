#pragma once
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include "ESPressio_ClockTypes.hpp"
#include "ESPressio_TimeReliability.hpp"
#include "ESPressio_ClockUncertainty.hpp"
namespace ESPressio::Timing {
/// <summary>Capture evidence, distinct from the clock's semantic reliability. Zero is invalid input.</summary>
enum class ClockCaptureQuality : std::uint8_t { Invalid=0, Hardware=1, SoftwareBounded=2, SoftwareUnbounded=3 };
/// <summary>Capture-consistent System and raw monotonic coordinates, with a physical capture-error bound.</summary>
template<class TTick=ClockTick> struct ClockTimestampCapture final {
    TTick SystemTimeNanoseconds=0;
    std::uint64_t MonotonicTimeNanoseconds=0;
    ClockUncertainty Uncertainty{};
    ClockCaptureQuality Quality=ClockCaptureQuality::Invalid;
};
/// <summary>Complete four-capture exchange and trusted reference context; transport never computes qualification.</summary>
template<class TTick=ClockTick> struct ClockSynchronizationObservation final {
    ClockTimestampCapture<TTick> T1{},T2{},T3{},T4{};
    std::uint64_t ReferenceIdentity=0;
    TimeReliability ReferenceReliability=TimeReliability::Unqualified;
    ClockUncertainty ReferenceUncertainty{};
    /// <summary>Only a trusted calibrated/authenticated path proof may replace the default ceil(RTT/2).</summary>
    bool HasCalibratedAsymmetryBound=false;
    std::uint64_t CalibratedAsymmetryBoundNanoseconds=0;
};
/// <summary>Finite composition profile. Numeric defaults are not a hardware certification.</summary>
struct ClockSynchronizationProfile final {
    static constexpr std::uint64_t QualifiedCeilingNanoseconds=1000000;
    static constexpr std::uint64_t EntryUncertaintyNanoseconds=500000;
    std::uint64_t MaximumAcceptedRoundTripDelayNanoseconds=100000000;
    std::uint64_t MaximumCaptureUncertaintyNanoseconds=10000000;
    std::uint64_t MinimumRegressionObservationSpanNanoseconds=500000000;
    std::size_t MinimumAcceptedSamples=4;
    std::uint64_t UncertaintyWeightFloorNanoseconds=1000;
    std::uint64_t ResidualEnvelopeNanoseconds=10000;
    std::uint32_t ResidualEnvelopeUncertaintyMultiplier=3;
    double ResidualFrequencyErrorBoundPpm=50;
    double MaximumFrequencyCorrectionPpm=2000;
    std::uint32_t MaximumSlewRatePpm=500;
    std::uint64_t QuantizationGuardNanoseconds=1;
    std::uint64_t OperationalGuardNanoseconds=250000;
    std::uint64_t AcquisitionIntervalNanoseconds=250000000;
    std::uint64_t MinimumExchangeIntervalNanoseconds=10000000;
    std::uint64_t MaximumExchangeIntervalNanoseconds=10000000000;
    std::uint64_t MaximumFreshSampleAgeNanoseconds=10000000000;
    bool AllowUnqualifiedReferenceForAcquisition=true;
    /// <summary>Rejects impossible/unsafe tuning rather than clamping it into a different profile.</summary>
    bool IsValid(std::size_t capacity) const noexcept {
        return capacity>=4 && MinimumAcceptedSamples>=4 && MinimumAcceptedSamples<=capacity &&
            MaximumAcceptedRoundTripDelayNanoseconds>0 && MaximumCaptureUncertaintyNanoseconds>0 &&
            MinimumRegressionObservationSpanNanoseconds>0 && UncertaintyWeightFloorNanoseconds>0 &&
            ResidualEnvelopeUncertaintyMultiplier>0 && std::isfinite(ResidualFrequencyErrorBoundPpm) &&
            ResidualFrequencyErrorBoundPpm>=0 && ResidualFrequencyErrorBoundPpm<999999 &&
            std::isfinite(MaximumFrequencyCorrectionPpm) && MaximumFrequencyCorrectionPpm>=0 &&
            MaximumSlewRatePpm>0 && MaximumFrequencyCorrectionPpm+MaximumSlewRatePpm<1000000 &&
            std::ceil(MaximumFrequencyCorrectionPpm*1000)+static_cast<double>(MaximumSlewRatePpm)*1000<1000000000 &&
            QuantizationGuardNanoseconds>0 && OperationalGuardNanoseconds<QualifiedCeilingNanoseconds && MinimumExchangeIntervalNanoseconds>0 &&
            MaximumExchangeIntervalNanoseconds>=MinimumExchangeIntervalNanoseconds &&
            AcquisitionIntervalNanoseconds>=MinimumExchangeIntervalNanoseconds &&
            AcquisitionIntervalNanoseconds<=MaximumExchangeIntervalNanoseconds && MaximumFreshSampleAgeNanoseconds>0;
    }
};
/// <summary>Reason-specific bounded counters distinguish malformed, physically unqualified and outlying evidence.</summary>
enum class ClockObservationRejection : std::uint8_t {
    None,InvalidTimestampOrder,RemoteProcessingExceedsLocalElapsed,RoundTripDelayExceeded,
    InvalidCaptureQuality,CaptureUncertaintyExceeded,InvalidReference,ReferenceMismatch,
    UnqualifiedReference,NumericOverflow,NonIncreasingObservation,Outlier,InvalidProfile,Count
};
struct ClockObservationValidation final {
    ClockObservationRejection Rejection=ClockObservationRejection::None;
    std::uint64_t LocalElapsedNanoseconds=0,RemoteProcessingElapsedNanoseconds=0,RoundTripDelayNanoseconds=0;
    std::uint64_t ObservationMonotonicNanoseconds=0;
    std::int64_t MeasuredOffsetNanoseconds=0;
    /// <summary>Capture-consistent normalized reference-minus-monotonic offset, immune to intervening local servo changes.</summary>
    std::int64_t ReferenceMinusMonotonicNanoseconds=0;
    ClockUncertainty Uncertainty{};
    bool ReferenceQualified=false;
    constexpr bool Accepted() const noexcept { return Rejection==ClockObservationRejection::None; }
};
/// <summary>Validates original captures and computes Timing-owned offset/delay/uncertainty without changing estimator state.</summary>
template<class TTick> ClockObservationValidation ValidateClockSynchronizationObservation(
    const ClockSynchronizationObservation<TTick>& o,const ClockSynchronizationProfile& profile) noexcept {
    ClockObservationValidation r;
    auto fail=[&](ClockObservationRejection reason) { r.Rejection=reason; return r; };
    if (!o.ReferenceIdentity || !IsValidTimeReliability(o.ReferenceReliability)) return fail(ClockObservationRejection::InvalidReference);
    bool captureKnown=true; std::uint64_t captureTotal=0;
    for (const auto* capture:std::array<const ClockTimestampCapture<TTick>*,4>{&o.T1,&o.T2,&o.T3,&o.T4}) {
        if (capture->Quality==ClockCaptureQuality::Invalid || capture->Quality>ClockCaptureQuality::SoftwareUnbounded)
            return fail(ClockObservationRejection::InvalidCaptureQuality);
        if (capture->Quality==ClockCaptureQuality::SoftwareUnbounded || !capture->Uncertainty.IsKnown) captureKnown=false;
        else {
            if (capture->Uncertainty.Nanoseconds>profile.MaximumCaptureUncertaintyNanoseconds)
                return fail(ClockObservationRejection::CaptureUncertaintyExceeded);
            if (capture->Uncertainty.Nanoseconds>UINT64_MAX-captureTotal) return fail(ClockObservationRejection::NumericOverflow);
            captureTotal+=capture->Uncertainty.Nanoseconds;
        }
    }
    if (o.T4.SystemTimeNanoseconds<o.T1.SystemTimeNanoseconds || o.T3.SystemTimeNanoseconds<o.T2.SystemTimeNanoseconds ||
        o.T4.MonotonicTimeNanoseconds<o.T1.MonotonicTimeNanoseconds || o.T3.MonotonicTimeNanoseconds<o.T2.MonotonicTimeNanoseconds)
        return fail(ClockObservationRejection::InvalidTimestampOrder);
    // RTT follows the locked four-System-timestamp equation. Raw monotonic captures
    // independently establish the regression coordinate and exclude later-time reconstruction.
    r.LocalElapsedNanoseconds=o.T4.SystemTimeNanoseconds-o.T1.SystemTimeNanoseconds;
    r.RemoteProcessingElapsedNanoseconds=o.T3.SystemTimeNanoseconds-o.T2.SystemTimeNanoseconds;
    if (r.RemoteProcessingElapsedNanoseconds>r.LocalElapsedNanoseconds) return fail(ClockObservationRejection::RemoteProcessingExceedsLocalElapsed);
    r.RoundTripDelayNanoseconds=r.LocalElapsedNanoseconds-r.RemoteProcessingElapsedNanoseconds;
    if (!profile.MaximumAcceptedRoundTripDelayNanoseconds || r.RoundTripDelayNanoseconds>profile.MaximumAcceptedRoundTripDelayNanoseconds)
        return fail(ClockObservationRejection::RoundTripDelayExceeded);
    std::int64_t a=0,b=0;
    if (!ClockMath::Difference(o.T2.SystemTimeNanoseconds,o.T1.SystemTimeNanoseconds,a) ||
        !ClockMath::Difference(o.T3.SystemTimeNanoseconds,o.T4.SystemTimeNanoseconds,b)) return fail(ClockObservationRejection::NumericOverflow);
    r.MeasuredOffsetNanoseconds=ClockMath::Average(a,b);
    r.ObservationMonotonicNanoseconds=ClockMath::Midpoint(o.T1.MonotonicTimeNanoseconds,o.T4.MonotonicTimeNanoseconds);
    const auto referenceMid=ClockMath::Midpoint(o.T2.SystemTimeNanoseconds,o.T3.SystemTimeNanoseconds);
    if (!ClockMath::Difference(referenceMid,r.ObservationMonotonicNanoseconds,r.ReferenceMinusMonotonicNanoseconds))
        return fail(ClockObservationRejection::NumericOverflow);
    r.ReferenceQualified=IsQualifiedTimeReliability(o.ReferenceReliability) && o.ReferenceUncertainty.IsKnown &&
        o.ReferenceUncertainty.Nanoseconds<ClockSynchronizationProfile::QualifiedCeilingNanoseconds;
    if (!r.ReferenceQualified && !profile.AllowUnqualifiedReferenceForAcquisition) return fail(ClockObservationRejection::UnqualifiedReference);
    if (captureKnown && o.ReferenceUncertainty.IsKnown) {
        const auto path=o.HasCalibratedAsymmetryBound ? o.CalibratedAsymmetryBoundNanoseconds : ClockMath::HalfCeiling(r.RoundTripDelayNanoseconds);
        const auto capture=ClockMath::HalfCeiling(captureTotal);
        if (path>UINT64_MAX-capture || o.ReferenceUncertainty.Nanoseconds>UINT64_MAX-path-capture)
            return fail(ClockObservationRejection::NumericOverflow);
        r.Uncertainty=ClockUncertainty::Known(o.ReferenceUncertainty.Nanoseconds+path+capture);
    }
    return r;
}
/// <summary>Fixed observation result; acceptance does not itself imply synchronized qualification.</summary>
struct ClockSynchronizationResult final {
    bool Accepted=false;
    ClockObservationRejection Rejection=ClockObservationRejection::None;
    std::int64_t MeasuredOffsetNanoseconds=0,ModelPhaseResidualNanoseconds=0;
    std::uint64_t RoundTripDelayNanoseconds=0;
    double EstimatedFrequencyCorrectionPpm=0;
};
/// <summary>Fixed diagnostic snapshot; expiry is evaluated against raw monotonic time without getter callbacks.</summary>
struct ClockSynchronizationStatus final {
    TimeReliability Reliability=TimeReliability::Unqualified;
    std::uint64_t ReferenceIdentity=0,LastAcceptedSampleMonotonic=0,SampleAgeNanoseconds=0;
    std::uint64_t LastRoundTripDelayNanoseconds=0,ModelResidualEnvelopeNanoseconds=0;
    std::int64_t LastMeasuredOffsetNanoseconds=0,ModelPhaseResidualNanoseconds=0,PendingPhaseSlewNanoseconds=0;
    double EstimatedFrequencyCorrectionPpm=0,ResidualFrequencyErrorBoundPpm=0;
    ClockUncertainty CurrentUncertainty{};
    std::size_t RetainedSamples=0,InlierSamples=0;
    std::uint64_t ObservationSpanNanoseconds=0,AcceptedSamples=0,RejectedSamples=0;
    std::array<std::uint64_t,static_cast<std::size_t>(ClockObservationRejection::Count)> RejectedByReason{};
    std::array<std::uint64_t,4> CaptureQualityCounts{};
    std::uint64_t SchedulerDeadlineMisses=0;
    bool HasSynchronizationDeadline=false;
    std::uint64_t NextRequiredSynchronizationMonotonic=0;
};
/// <summary>Bootstrap/reconfiguration outcomes never silently weaken continuity or profile requirements.</summary>
enum class ClockConfigurationStatus : std::uint8_t { Success,InvalidProfile,ContinuitySealed,InvalidReference,InvalidCoordinate };
}
