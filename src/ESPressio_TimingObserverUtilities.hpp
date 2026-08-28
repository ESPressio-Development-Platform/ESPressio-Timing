#pragma once

#include <cstdint>
#include <limits>

namespace ESPressio {
namespace Timing {
namespace Internal {

/// <summary>Computes a signed, saturating difference between two unsigned clock tick values.</summary>
template<typename TTick>
inline int64_t SignedDifference(
    TTick after,
    TTick before
) {
    if (after >= before) {
        const uint64_t difference =
            static_cast<uint64_t>(after - before);

        return difference >
            static_cast<uint64_t>(
                std::numeric_limits<int64_t>::max()
            )
            ? std::numeric_limits<int64_t>::max()
            : static_cast<int64_t>(difference);
    }

    const uint64_t difference =
        static_cast<uint64_t>(before - after);

    return difference >
        static_cast<uint64_t>(
            std::numeric_limits<int64_t>::max()
        )
        ? std::numeric_limits<int64_t>::min()
        : -static_cast<int64_t>(difference);
}

} // namespace Internal
} // namespace Timing
} // namespace ESPressio
