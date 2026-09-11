#include <ESPressio_Timing.hpp>
#include "CounterTestProvider.hpp"
#include <cassert>
using namespace ESPressio::Timing;
int main() {
    TestMonotonic monotonic; monotonic.Now=1234;
    CounterTestProvider provider; provider.FailCreate=true;
    ESPressio::System::Clock::SetMonotonicClock(&monotonic);
    ESPressio::System::Clock::SetHighResolutionCounterProvider(&provider);
    auto* source=HighResolutionTimeSource::GetInstance();
    assert(!source->GetIsUsingHighResolutionCounter() && source->GetTicksPerSecond()==1000000000);
    assert(source->GetTicks()==1234);
    GPTimerClock<> absent(true); assert(!absent.GetIsAvailable() && !absent.GetIsRunning());
    provider.FailCreate=false; provider.FailStart=true;
    GPTimerClock<> failed; assert(!failed.GetIsAvailable());
    assert(failed.GetInitializationResult().Status==ESPressio::System::PlatformStatus::HardwareFailure);
    ESPressio::System::Clock::ResetHighResolutionCounterProvider();
    ESPressio::System::Clock::ResetMonotonicClock();
}
