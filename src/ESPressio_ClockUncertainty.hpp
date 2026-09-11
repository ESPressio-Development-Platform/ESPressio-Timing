#pragma once
#include <cstdint>
#include <limits>
#include <cmath>
namespace ESPressio::Timing {
/// <summary>Unknown physical error is explicit and can never be used as a finite certification bound.</summary>
struct ClockUncertainty final {
    bool IsKnown=false;
    std::uint64_t Nanoseconds=0;
    static constexpr ClockUncertainty Known(std::uint64_t value) noexcept { return {true,value}; }
};
namespace ClockMath {
constexpr std::uint64_t Add(std::uint64_t a,std::uint64_t b) noexcept {
    return b>UINT64_MAX-a ? UINT64_MAX : a+b;
}
constexpr std::uint64_t Abs(std::int64_t value) noexcept {
    return value>=0 ? static_cast<std::uint64_t>(value) : static_cast<std::uint64_t>(-(value+1))+1;
}
constexpr std::uint64_t HalfCeiling(std::uint64_t value) noexcept { return value/2+value%2; }
constexpr std::int64_t Average(std::int64_t a,std::int64_t b) noexcept {
    auto whole=a/2+b/2;
    const auto remainder=a%2+b%2;
    whole+=remainder/2;
    if (remainder%2>0 && whole<0) ++whole;
    if (remainder%2<0 && whole>0) --whole;
    return whole;
}
constexpr std::uint64_t Midpoint(std::uint64_t a,std::uint64_t b) noexcept { return a+(b-a)/2; }
/// <summary>Computes elapsed * rate / 1e9 without a 128-bit intermediate; rate must not exceed 1e9.</summary>
constexpr std::uint64_t ScalePartsPerBillion(std::uint64_t elapsed,std::uint32_t rate,bool roundUp=false) noexcept {
    if (rate>1000000000u) return UINT64_MAX;
    const auto whole=(elapsed/1000000000ull)*rate;
    const auto product=(elapsed%1000000000ull)*rate;
    return Add(whole,product/1000000000ull+(roundUp && product%1000000000ull ? 1 : 0));
}
constexpr std::uint64_t Apply(std::uint64_t value,std::int64_t correction) noexcept {
    return correction>=0 ? Add(value,static_cast<std::uint64_t>(correction)) :
        (Abs(correction)>value ? 0 : value-Abs(correction));
}
/// <summary>Rejects an unrepresentable signed difference rather than fabricating a clipped observation.</summary>
constexpr bool Difference(std::uint64_t a,std::uint64_t b,std::int64_t& output) noexcept {
    if (a>=b) { if (a-b>static_cast<std::uint64_t>(INT64_MAX)) return false; output=static_cast<std::int64_t>(a-b); }
    else { const auto magnitude=b-a; if (magnitude>static_cast<std::uint64_t>(INT64_MAX)+1) return false;
        output=magnitude==static_cast<std::uint64_t>(INT64_MAX)+1 ? INT64_MIN : -static_cast<std::int64_t>(magnitude); }
    return true;
}
inline std::int64_t Signed(long double value) noexcept {
    if (!std::isfinite(value)) return value<0 ? INT64_MIN : INT64_MAX;
    if (value>=static_cast<long double>(INT64_MAX)) return INT64_MAX;
    if (value<=static_cast<long double>(INT64_MIN)) return INT64_MIN;
    return static_cast<std::int64_t>(value);
}
inline std::uint64_t Ceiling(long double value) noexcept {
    if (!std::isfinite(value) || value>=static_cast<long double>(UINT64_MAX)) return UINT64_MAX;
    if (value<=0) return 0;
    return static_cast<std::uint64_t>(std::ceil(value));
}
}
}
