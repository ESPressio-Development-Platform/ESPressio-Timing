#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include "ESPressio_ClockSynchronization.hpp"
namespace ESPressio::Timing {
/// <summary>One bounded observation, normalized using original capture coordinates rather than a later System-clock read.</summary>
struct ClockRegressionObservation final {
    std::uint64_t Monotonic=0;
    std::int64_t MeasuredOffset=0;
    std::int64_t ReferenceMinusMonotonic=0;
    ClockUncertainty Uncertainty{};
    bool QualifiedReference=false;
};
/// <summary>Exactly two weighted affine fits with a fixed residual-based rejection pass between them.</summary>
template<std::size_t N=8> class ClockRegression final {
    static_assert(N>=4,"Clock regression requires at least four fixed observations");
    std::array<ClockRegressionObservation,N> _samples{};
    std::size_t _count=0,_next=0;
public:
    struct Fit final {
        bool Valid=false,AllEvidenceQualified=false;
        std::uint64_t AnchorMonotonic=0,ObservationSpan=0;
        long double Intercept=0,Slope=0;
        std::size_t Inliers=0;
        std::array<bool,N> Included{};
        std::uint64_t MaximumObservationUncertainty=0,ResidualEnvelope=0;
        long double Predict(std::uint64_t now) const noexcept {
            const auto delta=now>=AnchorMonotonic ? static_cast<long double>(now-AnchorMonotonic) : -static_cast<long double>(AnchorMonotonic-now);
            return Intercept+Slope*delta;
        }
    };
    void Clear() noexcept { _samples={}; _count=0; _next=0; }
    std::size_t Size() const noexcept { return _count; }
    std::size_t NextIndex() const noexcept { return _next; }
    const ClockRegressionObservation& At(std::size_t i) const noexcept { return _samples[i]; }
    /// <summary>Caller validates increasing coordinates/reference lineage; overwrite is fixed-window rollover.</summary>
    void Add(const ClockRegressionObservation& value) noexcept {
        _samples[_next]=value; _next=(_next+1)%N; if (_count<N) ++_count;
    }
    Fit Calculate(const ClockSynchronizationProfile& profile) const noexcept {
        Fit result;
        if (!_count) return result;
        result.AnchorMonotonic=_samples[(_next+N-1)%N].Monotonic;
        std::array<long double,N> x{},y{},weight{};
        std::array<bool,N> included{};
        // Center both coordinates. Converting a full epoch to floating point would
        // lose low nanosecond bits on targets where long double is IEEE binary64.
        const auto yAnchor=_samples[(_next+N-1)%N].ReferenceMinusMonotonic;
        auto signedDelta=[](std::int64_t a,std::int64_t b) noexcept {
            if ((a<0)==(b<0)) return static_cast<long double>(a-b);
            return static_cast<long double>(a)-static_cast<long double>(b);
        };
        for (std::size_t i=0;i<_count;++i) {
            x[i]=_samples[i].Monotonic>=result.AnchorMonotonic ? static_cast<long double>(_samples[i].Monotonic-result.AnchorMonotonic) :
                -static_cast<long double>(result.AnchorMonotonic-_samples[i].Monotonic);
            y[i]=signedDelta(_samples[i].ReferenceMinusMonotonic,yAnchor);
            const auto uncertainty=_samples[i].Uncertainty.IsKnown ? _samples[i].Uncertainty.Nanoseconds :
                ClockSynchronizationProfile::QualifiedCeilingNanoseconds;
            const auto u=static_cast<long double>(std::max(uncertainty,profile.UncertaintyWeightFloorNanoseconds));
            // Common floor scaling leaves the least-squares solution unchanged while avoiding extreme tiny weights.
            const auto ratio=static_cast<long double>(profile.UncertaintyWeightFloorNanoseconds)/u;
            weight[i]=ratio*ratio; included[i]=true;
        }
        auto fit=[&](long double& intercept,long double& slope) noexcept {
            long double sw=0,sx=0,sy=0;
            for (std::size_t i=0;i<_count;++i) if (included[i]) { sw+=weight[i]; sx+=weight[i]*x[i]; sy+=weight[i]*y[i]; }
            if (!(sw>0) || !std::isfinite(sw)) return false;
            const auto mx=sx/sw,my=sy/sw;
            long double xx=0,xy=0;
            for (std::size_t i=0;i<_count;++i) if (included[i]) { const auto dx=x[i]-mx; xx+=weight[i]*dx*dx; xy+=weight[i]*dx*(y[i]-my); }
            slope=xx>0 ? xy/xx : 0; intercept=my-slope*mx;
            return std::isfinite(intercept) && std::isfinite(slope);
        };
        long double intercept=0,slope=0;
        if (!fit(intercept,slope)) return result;
        for (std::size_t i=0;i<_count;++i) {
            const auto uncertainty=_samples[i].Uncertainty.IsKnown ? _samples[i].Uncertainty.Nanoseconds : ClockSynchronizationProfile::QualifiedCeilingNanoseconds;
            const auto envelope=static_cast<long double>(profile.ResidualEnvelopeNanoseconds)+
                static_cast<long double>(profile.ResidualEnvelopeUncertaintyMultiplier)*std::max(uncertainty,profile.UncertaintyWeightFloorNanoseconds);
            included[i]=std::fabs(y[i]-intercept-slope*x[i])<=envelope;
        }
        // Exactly one second solve, even when the retained set is insufficient for qualification.
        if (!fit(intercept,slope)) return result;
        result.Valid=true; result.Intercept=static_cast<long double>(yAnchor)+intercept; result.Slope=slope;
        result.AllEvidenceQualified=true;
        std::uint64_t oldest=UINT64_MAX,newest=0;
        for (std::size_t i=0;i<_count;++i) if (included[i]) {
            result.Included[i]=true; ++result.Inliers;
            oldest=std::min(oldest,_samples[i].Monotonic); newest=std::max(newest,_samples[i].Monotonic);
            result.AllEvidenceQualified=result.AllEvidenceQualified && _samples[i].QualifiedReference && _samples[i].Uncertainty.IsKnown;
            if (_samples[i].Uncertainty.IsKnown) result.MaximumObservationUncertainty=std::max(result.MaximumObservationUncertainty,_samples[i].Uncertainty.Nanoseconds);
            result.ResidualEnvelope=std::max(result.ResidualEnvelope,ClockMath::Ceiling(std::fabs(y[i]-intercept-slope*x[i])));
        }
        result.ObservationSpan=newest-oldest;
        return result;
    }
};
}
