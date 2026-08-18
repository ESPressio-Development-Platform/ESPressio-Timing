#include <cassert>
#include <cstdint>
#include <type_traits>

#include <ESPressio_Timing.hpp>
#include "SerializableTime.hpp"

using namespace ESPressio;
using namespace ESPressio::Timing;

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
