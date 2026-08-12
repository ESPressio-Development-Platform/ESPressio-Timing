#include <cassert>
#include <type_traits>

#include "ESPressio_Timing.hpp"

#if !ESPRESSIO_TIMING_HAS_GPTIMER
    #error "The GPTimer test requires GPTimer availability"
#endif

using namespace ESPressio::Timing;

static_assert(
    std::is_base_of<IClock, GPTimerClock>::value,
    "GPTimerClock must implement IClock"
);

int main() {
    HighResolutionTimeSource* defaultSource =
        HighResolutionTimeSource::GetInstance();

    assert(defaultSource->GetIsUsingGPTimer());
    assert(defaultSource->GetTicksPerSecond() == 10000000UL);

    GPTimerClock clock(true, 10000000UL);

    assert(clock.GetIsAvailable());
    assert(clock.GetInitializationResult() == ESP_OK);
    assert(clock.GetResolution().value == 100);
    assert(
        clock.GetResolution().orderOfMagnitude ==
            ESPressio::Units::Nano
    );

    const IClock& clockInterface = clock;
    assert(clockInterface.GetTime().context ==
        ESPressio::Units::UnitContext::Time);
    return 0;
}
