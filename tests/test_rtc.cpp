#include <cassert>

#include <ESPressio_Timing.hpp>

using namespace ESPressio;
using namespace ESPressio::Timing;

class FakeSource : public ITimeSource {
    public:
        uint64_t ticks = 0;

        uint64_t GetTicks() const override {
            return ticks;
        }

        uint64_t GetTicksPerSecond() const override {
            return 1000000ULL;
        }
};

class FakeRTC :
    public RTCClockBase<
        DefaultClockTime,
        NoLockPolicy
    > {

    public:
        DefaultClockTime stored {
            10,
            Units::Base
        };

        FakeRTC(
            FakeSource* source
        )
            : RTCClockBase(
                DefaultClockTime(
                    1,
                    Units::Base
                ),
                source
            ) {
        }

    protected:
        bool ReadRTC(
            DefaultClockTime& time
        ) override {
            time = stored;
            return true;
        }

        bool WriteRTC(
            const DefaultClockTime& time
        ) override {
            stored = time;
            return true;
        }
};

int main() {
    FakeSource source;
    FakeRTC rtc(&source);

    assert(rtc.Synchronize());

    source.ticks = 500000;

    auto now = rtc.GetTime();

    // 10.5s represented at RTC resolution (seconds)
    assert(now.orderOfMagnitude == Units::Base);
    assert(now.value == 10); // resolution intentionally truncates to whole seconds

    return 0;
}
