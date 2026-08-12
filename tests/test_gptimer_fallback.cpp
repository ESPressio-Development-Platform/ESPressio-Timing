#include <cassert>

#include "ESPressio_Timing.hpp"

#if !ESPRESSIO_TIMING_HAS_GPTIMER
    #error "The fallback test requires compile-time GPTimer availability"
#endif

using namespace ESPressio::Timing;

int main() {
    HighResolutionTimeSource* source =
        HighResolutionTimeSource::GetInstance();

    assert(!source->GetIsUsingGPTimer());
    assert(source->GetTicksPerSecond() == 1000000ULL);
    assert(source->GetTicks() == 0);
    return 0;
}
