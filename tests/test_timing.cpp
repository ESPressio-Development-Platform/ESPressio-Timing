#include <cassert>
#include <atomic>
#include <cstdint>
#include <thread>
#include <type_traits>
#include <vector>

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
static_assert(
    std::is_base_of<IClock, SingleThreadedSystemClock>::value,
    "SingleThreadedSystemClock must implement IClock"
);
static_assert(
    std::is_base_of<IClock, SingleThreadedStopwatchClock>::value,
    "SingleThreadedStopwatchClock must implement IClock"
);
static_assert(
    std::is_base_of<IClock, SingleThreadedRTCClockBase>::value,
    "SingleThreadedRTCClockBase must implement IClock"
);
static_assert(
    !std::is_same<
        StopwatchClock,
        SingleThreadedStopwatchClock
    >::value,
    "Thread-safe and single-threaded clocks must be distinct types"
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

class ConcurrentTimeSource : public ITimeSource {
    public:
        std::atomic<uint64_t> ticks{0};
        mutable std::atomic<uint32_t> readCount{0};

        uint64_t GetTicks() const override {
            const uint64_t capturedTicks = ticks.load(
                std::memory_order_acquire
            );
            readCount.fetch_add(1, std::memory_order_release);
            return capturedTicks;
        }

        uint64_t GetTicksPerSecond() const override {
            return 1000000ULL;
        }
};

class LockableStopwatchClock : public StopwatchClock {
    public:
        explicit LockableStopwatchClock(ITimeSource* source)
            : StopwatchClock(true, source) { }

        void LockState() {
            _clockMutex.lock();
        }

        void UnlockState() {
            _clockMutex.unlock();
        }
};

static ManualTimeSource& GetSystemTimeSource() {
    static ManualTimeSource source;
    return source;
}

static ManualTimeSource& GetSingleThreadedSystemTimeSource() {
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

class SingleThreadedTestRTCClock : public SingleThreadedRTCClockBase {
    public:
        ClockTime hardwareTime;

        explicit SingleThreadedTestRTCClock(ITimeSource* timeSource)
            : SingleThreadedRTCClockBase(
                ClockTime(1, Units::Milli),
                timeSource
            ) { }

    protected:
        bool ReadRTC(ClockTime& time) override {
            time = hardwareTime;
            return true;
        }

        bool WriteRTC(ClockTime time) override {
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

static void TestMomentOfRequestUnderContention() {
    ConcurrentTimeSource source;
    LockableStopwatchClock stopwatch(&source);
    stopwatch.LockState();

    source.ticks.store(100, std::memory_order_release);
    const uint32_t initialReadCount = source.readCount.load(
        std::memory_order_acquire
    );
    ClockTime result;

    std::thread reader([&]() {
        result = stopwatch.GetTime();
    });

    while (
        source.readCount.load(std::memory_order_acquire) ==
            initialReadCount
    ) {
        std::this_thread::yield();
    }

    // The reader has captured 100 us but is waiting for the state lock.
    source.ticks.store(500, std::memory_order_release);
    stopwatch.UnlockState();
    reader.join();

    assert(result.value == 100);
    assert(result.orderOfMagnitude == Units::Micro);
}

static void TestConcurrentStopwatchAccess() {
    ConcurrentTimeSource source;
    StopwatchClock stopwatch(true, &source);
    std::vector<std::thread> workers;

    for (uint32_t worker = 0; worker < 4; ++worker) {
        workers.emplace_back([&]() {
            for (uint32_t iteration = 0; iteration < 1000; ++iteration) {
                source.ticks.fetch_add(1, std::memory_order_acq_rel);
                stopwatch.GetTime();

                if ((iteration % 31) == 0) {
                    stopwatch.Stop();
                    stopwatch.Start();
                }
            }
        });
    }

    for (std::thread& worker : workers) {
        worker.join();
    }

    assert(stopwatch.GetTime().orderOfMagnitude == Units::Micro);
}

static void TestSingleThreadedVariants() {
    assert(
        sizeof(SingleThreadedStopwatchClock) <
            sizeof(StopwatchClock)
    );

    ManualTimeSource source;
    SingleThreadedStopwatchClock stopwatch(true, &source);
    SingleThreadedTestRTCClock rtcClock(&source);

    source.ticks = 25;
    assert(stopwatch.GetTime().value == 25);
    assert(stopwatch.GetTime().orderOfMagnitude == Units::Micro);

    rtcClock.OnRTCInterrupt(ClockTime(2, Units::Milli));
    source.ticks = 1025;
    assert(rtcClock.GetTime().value == 3);
    assert(rtcClock.GetTime().orderOfMagnitude == Units::Milli);

    const IClock& stopwatchInterface = stopwatch;
    const IClock& rtcInterface = rtcClock;
    assert(stopwatchInterface.GetTime().context == Units::UnitContext::Time);
    assert(rtcInterface.GetTime().context == Units::UnitContext::Time);

    ManualTimeSource& systemSource =
        GetSingleThreadedSystemTimeSource();
    SingleThreadedSystemClock* systemClock =
        SingleThreadedSystemClock::GetInstance(&systemSource);
    systemClock->SetTime(ClockTime(10, Units::Micro));
    systemSource.ticks = 5;
    assert(systemClock->GetTime().value == 15);
}

int main() {
    TestTickConversion();
    TestStopwatch();
    TestRTC();
    TestSystemCallbacks();
    TestCommonClockInterface();
    TestMomentOfRequestUnderContention();
    TestConcurrentStopwatchAccess();
    TestSingleThreadedVariants();
    return 0;
}
