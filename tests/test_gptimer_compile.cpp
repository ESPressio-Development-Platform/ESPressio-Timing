#include <ESPressio_Timing.hpp>
#include "CounterTestProvider.hpp"
#include <cassert>
#include <type_traits>
using namespace ESPressio::Timing;
static_assert(std::is_base_of_v<IClock<>,GPTimerClock<>>);
static_assert(std::is_base_of_v<IClock<>,SingleThreadedGPTimerClock<>>);
int main() {
    CounterTestProvider provider;
    ESPressio::System::Clock::SetHighResolutionCounterProvider(&provider);
    auto* source=HighResolutionTimeSource::GetInstance();
    assert(source->GetIsUsingHighResolutionCounter() && source->GetTicksPerSecond()==10000000);
    GPTimerClock<> clock(true,10000000);
    assert(clock.GetIsAvailable() && clock.GetInitializationResult());
    assert(clock.GetResolution().value==100 && clock.GetResolution().orderOfMagnitude==ESPressio::Units::Nano);
    provider.Ticks=25;
    assert(clock.GetTime().value==2500);
    SingleThreadedGPTimerClock<> single(false,10000000); assert(single.GetIsAvailable());
    assert(provider.Created==3);
    ESPressio::System::Clock::ResetHighResolutionCounterProvider();
}
