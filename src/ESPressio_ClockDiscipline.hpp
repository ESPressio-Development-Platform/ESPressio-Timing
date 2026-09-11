#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include "ESPressio_ClockRegression.hpp"
#include "ESPressio_ClockModelSnapshot.hpp"
namespace ESPressio::Timing {
/// <summary>Owner-serialized K1/K2 discipline over a fixed window; transport, scheduling and callbacks remain outside it.</summary>
/// <remarks>Only accepted evidence replaces the uncertainty anchor. Pure model/status reads never mutate state.
/// The safety bound is physical: it is not divided by sample count or reduced by statistical confidence.</remarks>
template<std::size_t N=8> class ClockDiscipline final {
    static_assert(N>=4,"Clock discipline requires at least four fixed observations");
    ClockSynchronizationProfile _profile{};
    ClockRegression<N> _window;
    ClockModelSnapshot _model{};
    ClockSynchronizationStatus _diagnostics{};
    bool _sealed=false,_active=false,_referenceAvailable=false,_qualified=false,_mature=false,_latestQualified=false;
    std::uint64_t _reference=0,_lastObservation=0;
    bool _hasObservation=false;
    void ClearEvidence(std::uint64_t now) noexcept {
        const auto time=_model.Evaluate(now);
        const auto frequency=_model.FrequencyPartsPerBillion;
        _window.Clear(); _model={}; _model.AnchorMonotonic=now; _model.AnchorTime=time;
        _model.FrequencyPartsPerBillion=frequency;
        _hasObservation=false; _lastObservation=0; _qualified=false; _mature=false; _latestQualified=false;
        _diagnostics.RetainedSamples=0; _diagnostics.InlierSamples=0; _diagnostics.ObservationSpanNanoseconds=0;
        _diagnostics.HasSynchronizationDeadline=_active;
        _diagnostics.NextRequiredSynchronizationMonotonic=ClockMath::Add(now,_profile.AcquisitionIntervalNanoseconds);
    }
    void UpdateDeadline(std::uint64_t now) noexcept {
        _diagnostics.HasSynchronizationDeadline=_active;
        if (!_active) return;
        std::uint64_t interval=_profile.AcquisitionIntervalNanoseconds;
        const auto uncertainty=_model.UncertaintyAt(now);
        if (_qualified && uncertainty.IsKnown && uncertainty.Nanoseconds<ClockSynchronizationProfile::QualifiedCeilingNanoseconds) {
            const auto room=ClockSynchronizationProfile::QualifiedCeilingNanoseconds-uncertainty.Nanoseconds;
            if (room<=_profile.OperationalGuardNanoseconds) interval=0;
            else if (!_model.ResidualErrorPartsPerBillion) interval=_profile.MaximumExchangeIntervalNanoseconds;
            else {
                const std::uint64_t safe=(room-_profile.OperationalGuardNanoseconds)*1000000000ull/_model.ResidualErrorPartsPerBillion;
                // The minimum interval is a preferred cadence, never permission to schedule beyond safe headroom.
                interval=std::min(safe,_profile.MaximumExchangeIntervalNanoseconds);
                if (safe>=_profile.MinimumExchangeIntervalNanoseconds)
                    interval=std::max(interval,_profile.MinimumExchangeIntervalNanoseconds);
            }
            interval=std::min(interval,_profile.MaximumFreshSampleAgeNanoseconds);
        }
        _diagnostics.NextRequiredSynchronizationMonotonic=ClockMath::Add(now,interval);
    }
    ClockSynchronizationResult Reject(ClockObservationRejection reason) noexcept {
        ++_diagnostics.RejectedSamples; ++_diagnostics.RejectedByReason[static_cast<std::size_t>(reason)];
        ClockSynchronizationResult result; result.Rejection=reason; return result;
    }
public:
    ClockDiscipline() noexcept = default;
    /// <summary>Validates tuning transactionally, preserves current clock value and invalidates old qualification evidence.</summary>
    ClockConfigurationStatus Configure(const ClockSynchronizationProfile& profile,std::uint64_t now) noexcept {
        if (!profile.IsValid(N)) return ClockConfigurationStatus::InvalidProfile;
        _profile=profile; ClearEvidence(now); UpdateDeadline(now); return ClockConfigurationStatus::Success;
    }
    const ClockSynchronizationProfile& GetProfile() const noexcept { return _profile; }
    /// <summary>Establishes the one active trusted reference identity; a change clears samples without stepping the timeline.</summary>
    ClockConfigurationStatus SelectReference(std::uint64_t reference,std::uint64_t now) noexcept {
        if (!reference) return ClockConfigurationStatus::InvalidReference;
        if (reference==_reference) return ClockConfigurationStatus::Success;
        _reference=reference; _active=true; _referenceAvailable=true; ClearEvidence(now); UpdateDeadline(now);
        return ClockConfigurationStatus::Success;
    }
    /// <summary>Reports orchestration activity/availability without fabricating new timing evidence or reducing uncertainty.</summary>
    void SetActivity(bool acquiring,bool referenceAvailable,std::uint64_t now) noexcept {
        const bool newlyActive=acquiring && !_active;
        _active=acquiring; _referenceAvailable=referenceAvailable;
        if (newlyActive) UpdateDeadline(now);
        if (!_active) _diagnostics.HasSynchronizationDeadline=false;
    }
    /// <summary>Clears synchronization history while preserving the current continuous public coordinate and seal.</summary>
    void Reset(std::uint64_t now) noexcept { ClearEvidence(now); UpdateDeadline(now); }
    /// <summary>Explicit bootstrap-only rebase; a sealed runtime rejects it without changing any state.</summary>
    ClockConfigurationStatus TryRebase(std::uint64_t time,std::uint64_t now) noexcept {
        if (_sealed) return ClockConfigurationStatus::ContinuitySealed;
        ClearEvidence(now); _model={}; _model.AnchorMonotonic=now; _model.AnchorTime=time;
        UpdateDeadline(now); return ClockConfigurationStatus::Success;
    }
    void SealContinuity() noexcept { _sealed=true; }
    bool IsContinuitySealed() const noexcept { return _sealed; }
    const ClockModelSnapshot& Model() const noexcept { return _model; }
    void RecordSchedulerDeadlineMiss() noexcept { ++_diagnostics.SchedulerDeadlineMisses; }
    /// <summary>Validates a complete observation, stages a fixed candidate window and atomically commits accepted evidence.</summary>
    ClockSynchronizationResult Submit(const ClockSynchronizationObservation<>& observation,std::uint64_t now,
                                      std::uint64_t sourceQuantizationGuard=1) noexcept {
        if (!_profile.IsValid(N)) return Reject(ClockObservationRejection::InvalidProfile);
        const auto validated=ValidateClockSynchronizationObservation(observation,_profile);
        for (const auto* capture:std::array<const ClockTimestampCapture<>*,4>{&observation.T1,&observation.T2,&observation.T3,&observation.T4}) {
            const auto quality=static_cast<std::size_t>(capture->Quality);
            if (quality<_diagnostics.CaptureQualityCounts.size()) ++_diagnostics.CaptureQualityCounts[quality];
        }
        if (!validated.Accepted()) return Reject(validated.Rejection);
        if (!_reference || observation.ReferenceIdentity!=_reference) return Reject(ClockObservationRejection::ReferenceMismatch);
        if (observation.T4.MonotonicTimeNanoseconds>now || now<_model.AnchorMonotonic)
            return Reject(ClockObservationRejection::InvalidTimestampOrder);
        if (_hasObservation && validated.ObservationMonotonicNanoseconds<=_lastObservation)
            return Reject(ClockObservationRejection::NonIncreasingObservation);
        auto candidate=_window; // Fixed N-record staging, not a second live window or capacity fallback.
        const auto inserted=candidate.NextIndex();
        candidate.Add({validated.ObservationMonotonicNanoseconds,validated.MeasuredOffsetNanoseconds,
            validated.ReferenceMinusMonotonicNanoseconds,validated.Uncertainty,validated.ReferenceQualified});
        const auto fit=candidate.Calculate(_profile);
        if (!fit.Valid || !fit.Included[inserted]) return Reject(ClockObservationRejection::Outlier);
        const auto oldTime=_model.Evaluate(now);
        std::int64_t currentMinusMonotonic=0;
        if (!ClockMath::Difference(oldTime,now,currentMinusMonotonic)) return Reject(ClockObservationRejection::NumericOverflow);
        const auto targetOffset=fit.Predict(now);
        const auto phase=targetOffset-static_cast<long double>(currentMinusMonotonic);
        if (!std::isfinite(phase) || phase>static_cast<long double>(INT64_MAX) || phase<static_cast<long double>(INT64_MIN))
            return Reject(ClockObservationRejection::NumericOverflow);
        const auto desiredPpm=fit.Slope*1000000.0L;
        const auto frequencyPpm=std::clamp(desiredPpm,-static_cast<long double>(_profile.MaximumFrequencyCorrectionPpm),
                                        static_cast<long double>(_profile.MaximumFrequencyCorrectionPpm));
        ClockModelSnapshot model;
        model.AnchorMonotonic=now; model.AnchorTime=oldTime;
        model.FrequencyPartsPerBillion=static_cast<std::int32_t>(std::llround(frequencyPpm*1000));
        model.SlewPartsPerBillion=_profile.MaximumSlewRatePpm*1000;
        model.PendingPhaseNanoseconds=ClockMath::Signed(phase);
        const auto residualPpb=ClockMath::Ceiling(static_cast<long double>(_profile.ResidualFrequencyErrorBoundPpm)*1000+
            std::fabs(desiredPpm*1000-model.FrequencyPartsPerBillion)+1); // Include quantized/clipped-rate error; never reduce physical bound.
        if (residualPpb<=1000000000u) model.ResidualErrorPartsPerBillion=static_cast<std::uint32_t>(residualPpb);
        if (fit.AllEvidenceQualified && residualPpb<=1000000000u) {
            // Conservative floating arithmetic guard includes integer-to-floating conversion and cancellation
            // near large epochs, including targets where long double has only binary64 precision.
            const auto numericGuard=ClockMath::Ceiling(8*std::numeric_limits<long double>::epsilon()*
                (std::fabs(targetOffset)+std::fabs(static_cast<long double>(currentMinusMonotonic))+1));
            auto uncertainty=ClockMath::Add(fit.MaximumObservationUncertainty,fit.ResidualEnvelope);
            uncertainty=ClockMath::Add(uncertainty,ClockMath::Abs(model.PendingPhaseNanoseconds));
            uncertainty=ClockMath::Add(uncertainty,ClockMath::Add(_profile.QuantizationGuardNanoseconds,sourceQuantizationGuard));
            uncertainty=ClockMath::Add(uncertainty,numericGuard);
            uncertainty=ClockMath::Add(uncertainty,ClockMath::ScalePartsPerBillion(now-validated.ObservationMonotonicNanoseconds,
                                                                                              model.ResidualErrorPartsPerBillion,true));
            model.AnchorUncertainty=ClockUncertainty::Known(uncertainty);
        }
        const bool previouslyQualified=IsQualifiedTimeReliability(GetStatus(now).Reliability);
        _window=candidate; _model=model; _hasObservation=true; _lastObservation=validated.ObservationMonotonicNanoseconds;
        _latestQualified=validated.ReferenceQualified && validated.Uncertainty.IsKnown;
        _mature=fit.Inliers>=_profile.MinimumAcceptedSamples && fit.ObservationSpan>=_profile.MinimumRegressionObservationSpanNanoseconds;
        _qualified=_mature && _latestQualified && model.AnchorUncertainty.IsKnown &&
            (previouslyQualified ? model.AnchorUncertainty.Nanoseconds<ClockSynchronizationProfile::QualifiedCeilingNanoseconds :
                                   model.AnchorUncertainty.Nanoseconds<=ClockSynchronizationProfile::EntryUncertaintyNanoseconds);
        ++_diagnostics.AcceptedSamples;
        _diagnostics.LastMeasuredOffsetNanoseconds=validated.MeasuredOffsetNanoseconds;
        _diagnostics.ModelPhaseResidualNanoseconds=model.PendingPhaseNanoseconds;
        _diagnostics.LastRoundTripDelayNanoseconds=validated.RoundTripDelayNanoseconds;
        _diagnostics.ModelResidualEnvelopeNanoseconds=fit.ResidualEnvelope;
        _diagnostics.RetainedSamples=candidate.Size(); _diagnostics.InlierSamples=fit.Inliers;
        _diagnostics.ObservationSpanNanoseconds=fit.ObservationSpan;
        UpdateDeadline(now);
        return {true,ClockObservationRejection::None,validated.MeasuredOffsetNanoseconds,model.PendingPhaseNanoseconds,
                validated.RoundTripDelayNanoseconds,static_cast<double>(model.FrequencyPartsPerBillion)/1000};
    }
    /// <summary>Derives current qualification/holdover expiry without changing the window, model, counters or observers.</summary>
    ClockSynchronizationStatus GetStatus(std::uint64_t now) const noexcept {
        auto status=_diagnostics;
        status.ReferenceIdentity=_reference;
        status.LastAcceptedSampleMonotonic=_lastObservation;
        status.SampleAgeNanoseconds=_hasObservation && now>=_lastObservation ? now-_lastObservation : 0;
        status.CurrentUncertainty=_model.UncertaintyAt(now);
        status.PendingPhaseSlewNanoseconds=_model.PendingPhase(now);
        status.EstimatedFrequencyCorrectionPpm=static_cast<double>(_model.FrequencyPartsPerBillion)/1000;
        status.ResidualFrequencyErrorBoundPpm=_profile.ResidualFrequencyErrorBoundPpm;
        if (_qualified && status.CurrentUncertainty.IsKnown && status.CurrentUncertainty.Nanoseconds<ClockSynchronizationProfile::QualifiedCeilingNanoseconds) {
            const bool fresh=_referenceAvailable && _latestQualified && status.SampleAgeNanoseconds<=_profile.MaximumFreshSampleAgeNanoseconds &&
                (!_diagnostics.HasSynchronizationDeadline || now<=_diagnostics.NextRequiredSynchronizationMonotonic);
            status.Reliability=fresh ? TimeReliability::Synchronized : TimeReliability::Holdover;
        } else status.Reliability=_active ? TimeReliability::Acquiring : TimeReliability::Unqualified;
        return status;
    }
};
}
