#include <cassert>
#include <cstdint>
#include <type_traits>

#include "ESPressio_Timing.hpp"

using namespace ESPressio::Timing;
namespace Units = ESPressio::Units;

static_assert(
    std::is_base_of<IClock, SystemClock>::value,
    "SystemClock must implement IClock"
);
static_assert(
    std::is_base_of<IClock, StopwatchClock>::value,
    "StopwatchClock must implement IClock"
);
static_assert(
    std::is_base_of<IClock, RTCClockBase>::value,
    "RTCClockBase must implement IClock"
);
static_assert(
    ClockTime::context == ESPressio::Units::UnitContext::Time,
    "ClockTime must carry the ESPressio Units Time context"
);

class ManualTimeSource : public ITimeSource {
    public:
        uint64_t ticks = 0;
        uint64_t ticksPerSecond = 1000000ULL;

        uint64_t GetTicks() const override {
            return ticks;
        }

        uint64_t GetTicksPerSecond() const override {
            return ticksPerSecond;
        }
};

static ManualTimeSource& GetSystemTimeSource() {
    static ManualTimeSource source;
    return source;
}

class TestRTCClock : public RTCClockBase {
    public:
        ClockTime hardwareTime;
        bool canRead = true;
        bool canWrite = true;

        explicit TestRTCClock(ITimeSource* timeSource)
            : RTCClockBase(
                ClockTime(1, Units::Base),
                timeSource
            ) { }

    protected:
        bool ReadRTC(ClockTime& time) override {
            if (!canRead) {
                return false;
            }

            time = hardwareTime;
            return true;
        }

        bool WriteRTC(ClockTime time) override {
            if (!canWrite) {
                return false;
            }

            hardwareTime = time;
            return true;
        }
};

static void TestTickConversion() {
    assert(
        ESPressio::Timing::Internal::TicksToNanoseconds(1, 1000000) ==
            1000
    );
    assert(
        ESPressio::Timing::Internal::TicksToNanoseconds(3, 2) ==
            1500000000ULL
    );
    assert(
        ESPressio::Timing::Internal::GetSourceResolution(1000000) ==
            1000
    );
}

static void TestStopwatch() {
    ManualTimeSource source;
    StopwatchClock stopwatch(false, &source);

    assert(stopwatch.GetTime().value == 0);
    assert(!stopwatch.GetIsRunning());

    stopwatch.Start();
    source.ticks = 250;
    assert(stopwatch.GetTime().value == 250);
    assert(stopwatch.GetTime().orderOfMagnitude == Units::Micro);

    stopwatch.Stop();
    source.ticks = 500;
    assert(stopwatch.GetTime().value == 250);

    stopwatch.SetTime(ClockTime(1, Units::Micro));
    stopwatch.Restart();
    source.ticks = 510;
    assert(stopwatch.GetTime().value == 10);
    assert(stopwatch.GetTime().orderOfMagnitude == Units::Micro);

    stopwatch.Reset();
    assert(stopwatch.GetTime().value == 0);
}

static void TestRTC() {
    ManualTimeSource source;
    TestRTCClock clock(&source);

    assert(!clock.GetIsSynchronized());
    assert(clock.GetTime().value == 0);

    clock.hardwareTime = ClockTime(10, Units::Base);
    assert(clock.Synchronize());
    source.ticks = 250000;
    assert(clock.GetTime().value == 10);
    assert(clock.GetTime().orderOfMagnitude == Units::Base);
    assert(clock.GetResolution().value == 1);
    assert(clock.GetResolution().orderOfMagnitude == Units::Base);

    clock.OnRTCInterrupt(ClockTime(20, Units::Base));
    source.ticks = 500000;
    assert(clock.GetTime().value == 20);
    assert(clock.GetTime().orderOfMagnitude == Units::Base);

    clock.canWrite = false;
    assert(!clock.TrySetTime(ClockTime(30, Units::Base)));
    assert(clock.GetTime().value == 20);
}

static void TestSystemCallbacks() {
    ManualTimeSource& source = GetSystemTimeSource();
    SystemClock* clock = SystemClock::GetInstance(&source);
    bool invoked = false;

    clock->ClearCallbacks();
    clock->SetTime(ClockTime(0, Units::Micro));
    assert(clock->TrySetCallback(ClockTime(100, Units::Micro), [&invoked]() {
        invoked = true;
    }));
    source.ticks = 100;
    clock->Update();

    assert(invoked);
}

static ClockTime ReadClock(const IClock& clock) {
    return clock.GetTime();
}

static void TestCommonClockInterface() {
    ManualTimeSource& source = GetSystemTimeSource();
    StopwatchClock stopwatch(true, &source);
    TestRTCClock rtcClock(&source);
    SystemClock* systemClock = SystemClock::GetInstance(&source);

    systemClock->SetTime(ClockTime(1, Units::Micro));
    rtcClock.OnRTCInterrupt(ClockTime(2, Units::Base));
    source.ticks = 101;

    const IClock& systemClockInterface = *systemClock;
    const IClock& stopwatchInterface = stopwatch;
    const IClock& rtcClockInterface = rtcClock;

    assert(ReadClock(systemClockInterface).value == 2);
    assert(ReadClock(systemClockInterface).orderOfMagnitude == Units::Micro);
    assert(ReadClock(stopwatchInterface).value == 1);
    assert(ReadClock(stopwatchInterface).orderOfMagnitude == Units::Micro);
    assert(ReadClock(rtcClockInterface).value == 2);
    assert(ReadClock(rtcClockInterface).orderOfMagnitude == Units::Base);
}

int main() {
    TestTickConversion();
    TestStopwatch();
    TestRTC();
    TestSystemCallbacks();
    TestCommonClockInterface();
    return 0;
}
