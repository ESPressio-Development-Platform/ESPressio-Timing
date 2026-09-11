#include <ESPressio_Timing.hpp>
#include "CounterTestProvider.hpp"
#include <cassert>
int main() {
    TestMonotonic monotonic;
    ESPressio::System::Clock::SetMonotonicClock(&monotonic);
    ESPressio::System::Clock::ResetHighResolutionCounterProvider();
    auto& clock=ESPressio::Timing::SystemClock<>::GetInstance();
    monotonic.Now=0xfffffff0ull*1000;
    const auto before=clock.CaptureSynchronizationTimestamp();
    monotonic.Now=0x100000010ull*1000;
    const auto after=clock.CaptureSynchronizationTimestamp();
    assert(after.MonotonicTimeNanoseconds>before.MonotonicTimeNanoseconds);
    assert(after.Quality==ESPressio::Timing::ClockCaptureQuality::SoftwareUnbounded && !after.Uncertainty.IsKnown);
    assert(clock.GetResolution().value==1 && clock.GetResolution().orderOfMagnitude==ESPressio::Units::Micro);
    ESPressio::System::Clock::ResetMonotonicClock();
}
