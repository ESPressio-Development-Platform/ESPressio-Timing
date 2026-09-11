#include <ESPressio_Timing.hpp>
#include <atomic>
#include <array>
#include <thread>
#include <cassert>
using namespace ESPressio::Timing;
class Source final : public ITimeSource {
public:
    std::atomic<std::uint64_t> Ticks{0}; mutable std::atomic<unsigned> Reads{0};
    std::uint64_t GetTicks() const override { const auto value=Ticks.load(); ++Reads; return value; }
    std::uint64_t GetTicksPerSecond() const override { return 1000000; }
};
class LockedStopwatch final : public StopwatchClock<> {
public:
    explicit LockedStopwatch(Source& source):StopwatchClock<>(true,&source) {}
    void Lock() { _clockMutex.lock(); } void Unlock() { _clockMutex.unlock(); }
};
int main() {
    assert(Internal::TicksToNanoseconds(3,2)==1500000000ull);
    assert(Internal::TicksToNanoseconds(UINT64_MAX,1)==UINT64_MAX);
    assert(Internal::GetSourceResolution(1000000)==1000);
    Source source; LockedStopwatch clock(source);
    clock.Lock(); source.Ticks=100; const auto reads=source.Reads.load();
    DefaultClockTime value;
    std::thread reader([&]{value=clock.GetTime();});
    while (source.Reads==reads) std::this_thread::yield();
    source.Ticks=500; clock.Unlock(); reader.join();
    assert(value.value==100); // Request-time capture survives waiting for state ownership.
    std::array<std::thread,4> workers;
    for (auto& worker:workers) worker=std::thread([&]{
        for (unsigned i=0;i<1000;++i) { ++source.Ticks; (void)clock.GetTime(); if (i%31==0) { clock.Stop(); clock.Start(); } }
    });
    for (auto& worker:workers) worker.join();
    assert(clock.GetTime().orderOfMagnitude==ESPressio::Units::Micro);
}
