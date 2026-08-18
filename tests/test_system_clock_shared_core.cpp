#include <cassert>
#include <cstdint>

#include <ESPressio_Timing.hpp>
#include "SerializableTime.hpp"

using namespace ESPressio;
using namespace ESPressio::Timing;

class SharedFakeTimeSource :
    public ITimeSource {

    public:
        uint64_t ticks = 0;

        uint64_t GetTicks() const override {
            return ticks;
        }

        uint64_t GetTicksPerSecond() const override {
            return 1000000ULL;
        }
};


int main() {
    SharedFakeTimeSource source;

    using SerializableClockTime =
        Units::
            SerializableNanoSeconds<
                uint64_t
            >;

    using OrdinarySystemClock =
        SystemClock<
            DefaultClockTime,
            NoLockPolicy
        >;

    using SerializableSystemClock =
        SystemClock<
            SerializableClockTime,
            NoLockPolicy
        >;

    auto& ordinary =
        OrdinarySystemClock::
            GetInstance(
                &source
            );

    auto& serializable =
        SerializableSystemClock::
            GetInstance(
                &source
            );

    /*
     * Set through the ordinary facade.
     */
    ordinary.SetTime(
        DefaultClockTime(
            5,
            Units::Base
        )
    );

    source.ticks = 250000;

    /*
     * Read through the Serializable facade. Both facades must observe the same
     * underlying SystemClockCore.
     */
    SerializableClockTime now =
        serializable.GetTime();

    assert(
        now.orderOfMagnitude ==
        Units::Micro
    );

    assert(
        now.value ==
        5250000
    );

    /*
     * Set through the Serializable facade and read through the ordinary view.
     */
    serializable.SetTime(
        SerializableClockTime(
            10,
            Units::Base
        )
    );

    source.ticks = 500000;

    auto ordinaryNow =
        ordinary.GetTime();

    assert(
        ordinaryNow.orderOfMagnitude ==
        Units::Micro
    );

    assert(
        ordinaryNow.value ==
        10250000
    );

    /*
     * Callback registration and Update() are also shared by the one core.
     */
    bool fired = false;

    serializable.SetCallback(
        SerializableClockTime(
            11,
            Units::Base
        ),
        [&]() {
            fired = true;
        }
    );

    source.ticks = 1250000;

    /*
     * Update through the other facade; callback must still fire.
     */
    ordinary.Update();

    assert(fired);

    return 0;
}
