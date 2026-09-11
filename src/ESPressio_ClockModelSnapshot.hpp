#pragma once
#include <algorithm>
#include "ESPressio_ClockUncertainty.hpp"
namespace ESPressio::Timing {
/// <summary>Published continuous mapping from raw monotonic nanoseconds, with a finite piecewise-affine slew.</summary>
/// <remarks>At publication AnchorTime equals the preceding model evaluated at AnchorMonotonic. Slew never steps.
/// Frequency and slew bounds are validated to keep the effective rate positive. Integer evaluation avoids large
/// epoch floating-point loss and needs no 128-bit target arithmetic. Uncertainty keeps the anchor's full unresolved
/// phase budget until new evidence; elapsed slew alone never reduces a published safety bound.</remarks>
struct ClockModelSnapshot final {
    std::uint64_t AnchorMonotonic=0,AnchorTime=0;
    std::int32_t FrequencyPartsPerBillion=0;
    std::int64_t PendingPhaseNanoseconds=0;
    std::uint32_t SlewPartsPerBillion=0;
    ClockUncertainty AnchorUncertainty{};
    std::uint32_t ResidualErrorPartsPerBillion=0;
    /// <summary>Validates the published rate domain; callers must preserve these immutable snapshot invariants.</summary>
    bool IsValid() const noexcept { return ClockMath::Abs(FrequencyPartsPerBillion)+SlewPartsPerBillion<1000000000ull &&
        ResidualErrorPartsPerBillion<=1000000000u; }
    std::uint64_t Evaluate(std::uint64_t monotonic) const noexcept {
        const auto elapsed=monotonic>=AnchorMonotonic ? monotonic-AnchorMonotonic : 0;
        const auto phaseMagnitude=ClockMath::Abs(PendingPhaseNanoseconds);
        const auto removable=ClockMath::ScalePartsPerBillion(elapsed,SlewPartsPerBillion);
        auto subtract=[](std::uint64_t value,std::uint64_t amount) noexcept { return amount>value ? 0 : value-amount; };
        if (removable<phaseMagnitude) {
            // Round the combined affine rate once. Separately rounded negative
            // frequency and phase terms could otherwise jump backward by 1 ns.
            const auto combined=static_cast<std::int64_t>(FrequencyPartsPerBillion)+
                (PendingPhaseNanoseconds<0 ? -static_cast<std::int64_t>(SlewPartsPerBillion) : SlewPartsPerBillion);
            const auto correction=ClockMath::ScalePartsPerBillion(elapsed,static_cast<std::uint32_t>(ClockMath::Abs(combined)),combined<0);
            const auto progress=combined<0 ? subtract(elapsed,correction) : ClockMath::Add(elapsed,correction);
            return ClockMath::Add(AnchorTime,progress);
        }
        const bool frequencyNegative=FrequencyPartsPerBillion<0,phaseNegative=PendingPhaseNanoseconds<0;
        const auto frequency=ClockMath::ScalePartsPerBillion(elapsed,static_cast<std::uint32_t>(ClockMath::Abs(FrequencyPartsPerBillion)),frequencyNegative);
        std::uint64_t progress=elapsed;
        if (frequencyNegative==phaseNegative) {
            progress=frequencyNegative ? subtract(subtract(elapsed,frequency),phaseMagnitude) :
                ClockMath::Add(ClockMath::Add(elapsed,frequency),phaseMagnitude);
        } else if (frequency>=phaseMagnitude) {
            progress=frequencyNegative ? subtract(elapsed,frequency-phaseMagnitude) : ClockMath::Add(elapsed,frequency-phaseMagnitude);
        } else {
            progress=phaseNegative ? subtract(elapsed,phaseMagnitude-frequency) : ClockMath::Add(elapsed,phaseMagnitude-frequency);
        }
        return ClockMath::Add(AnchorTime,progress);
    }
    std::int64_t PendingPhase(std::uint64_t monotonic) const noexcept {
        const auto elapsed=monotonic>=AnchorMonotonic ? monotonic-AnchorMonotonic : 0;
        const auto removed=std::min(ClockMath::Abs(PendingPhaseNanoseconds),ClockMath::ScalePartsPerBillion(elapsed,SlewPartsPerBillion));
        if (PendingPhaseNanoseconds>=0) return PendingPhaseNanoseconds-static_cast<std::int64_t>(removed);
        const auto remaining=ClockMath::Abs(PendingPhaseNanoseconds)-removed;
        return remaining==static_cast<std::uint64_t>(INT64_MAX)+1 ? INT64_MIN : -static_cast<std::int64_t>(remaining);
    }
    ClockUncertainty UncertaintyAt(std::uint64_t monotonic) const noexcept {
        if (!AnchorUncertainty.IsKnown) return {};
        const auto elapsed=monotonic>=AnchorMonotonic ? monotonic-AnchorMonotonic : 0;
        return ClockUncertainty::Known(ClockMath::Add(AnchorUncertainty.Nanoseconds,
            ClockMath::ScalePartsPerBillion(elapsed,ResidualErrorPartsPerBillion,true)));
    }
};
}
