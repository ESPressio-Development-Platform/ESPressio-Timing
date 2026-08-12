#include "ESPressio_Timing.hpp"

int main() {
    ESPressio::Timing::StopwatchClock clock;
    return clock.GetResolution().value == 0;
}
