#include "ESPressio_Timing.hpp"

#if ESPRESSIO_TIMING_HAS_GPTIMER
    #error "GPTimer must not be exposed on a generic Arduino target"
#endif

int main() {
    ESPressio::Timing::StopwatchClock clock;
    return clock.GetResolution().value == 0;
}
