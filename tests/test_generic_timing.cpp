#include <cassert>
#include <cstdint>
#include <type_traits>

#include <ESPressio_Timing.hpp>
#include "SerializableTime.hpp"

using namespace ESPressio;
using namespace ESPressio::Timing;

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members:
 * - ticks (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - ticksPerSecond (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * Total Memory: 20 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class FakeTimeSource :
    public ITimeSource {

    public:
        uint64_t ticks = 0;
        uint64_t ticksPerSecond =
            1000000ULL;

        uint64_t GetTicks() const override {
            return ticks;
        }

        uint64_t GetTicksPerSecond() const override {
            return ticksPerSecond;
        }
};


int main() {
    FakeTimeSource source;

    {
        StopwatchClock<
            DefaultClockTime,
            NoLockPolicy
        > stopwatch(
            true,
            &source
        );

        source.ticks = 2500;

        const auto elapsed =
            stopwatch.GetTime();

        static_assert(
            std::is_same_v<
                decltype(elapsed),
                const DefaultClockTime
            >
        );

        assert(
            elapsed.orderOfMagnitude ==
            Units::Micro
        );

        assert(
            elapsed.value == 2500
        );
    }


    {
        using SerializableTime =
            Units::
                SerializableNanoSeconds<
                    uint64_t
                >;

        StopwatchClock<
            SerializableTime,
            NoLockPolicy
        > stopwatch(
            true,
            &source
        );

        source.ticks = 5000;

        SerializableTime elapsed =
            stopwatch.GetTime();

        assert(
            elapsed.orderOfMagnitude ==
            Units::Micro
        );

        assert(
            elapsed.value == 2500
        );

        static_assert(
            std::is_same_v<
                typename decltype(
                    stopwatch
                )::TimeType,
                SerializableTime
            >
        );
    }


    {
        using SmallTick =
            uint32_t;

        StopwatchClock<
            DefaultClockTime,
            NoLockPolicy,
            SmallTick
        > stopwatch(
            false,
            &source
        );

        static_assert(
            std::is_same_v<
                typename decltype(
                    stopwatch
                )::TickType,
                SmallTick
            >
        );
    }


    {
        IClock<DefaultClockTime>* clock =
            nullptr;

        static_assert(
            std::is_abstract_v<
                IClock<DefaultClockTime>
            >
        );

        (void)clock;
    }

    return 0;
}
