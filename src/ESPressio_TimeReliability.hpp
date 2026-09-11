#pragma once
#include <cstdint>
namespace ESPressio::Timing {
/// <summary>Canonical one-byte time labels; values are categorical, never an ordinal quality score.</summary>
enum class TimeReliability : std::uint8_t { Unqualified=0, Acquiring=1, Synchronized=2, Holdover=3 };
constexpr bool IsValidTimeReliability(TimeReliability value) noexcept {
    return value==TimeReliability::Unqualified || value==TimeReliability::Acquiring ||
           value==TimeReliability::Synchronized || value==TimeReliability::Holdover;
}
constexpr bool IsQualifiedTimeReliability(TimeReliability value) noexcept {
    return value==TimeReliability::Synchronized || value==TimeReliability::Holdover;
}
/// <summary>Semantic origin time captured as one value; receivers preserve both fields without upgrading quality.</summary>
struct QualifiedTime final {
    std::uint64_t Nanoseconds=0;
    TimeReliability Reliability=TimeReliability::Unqualified;
};
}
