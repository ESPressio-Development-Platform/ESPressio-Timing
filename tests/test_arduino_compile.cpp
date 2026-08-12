#include <cassert>

#include "ESPressio_Timing.hpp"

#if ESPRESSIO_TIMING_HAS_GPTIMER
    #error "GPTimer must not be exposed on a generic Arduino target"
#endif

int main() {
    ESPressio::Timing::StopwatchClock clock;
    ESPressio::Timing::HighResolutionTimeSource* source =
        ESPressio::Timing::HighResolutionTimeSource::GetInstance();

    ArduinoMicrosValue() = 0xFFFFFFF0UL;
    const uint64_t beforeRollover = source->GetTicks();
    ArduinoMicrosValue() = 0x00000010UL;
    const uint64_t afterRollover = source->GetTicks();

    assert(afterRollover > beforeRollover);
    assert(clock.GetResolution().value == 1);
    return 0;
}
