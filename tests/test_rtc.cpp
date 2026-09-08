#include <cassert>

#include <ESPressio_Timing.hpp>

using namespace ESPressio;
using namespace ESPressio::Timing;

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members:
 * - ticks (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * Total Memory: 12 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
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

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(ClockBase<TTime, TTick>) + sizeof(IRTCClock<TTime>) + 1 bytes known members + sizeof(TTick) + sizeof(TTick) + sizeof(TTick) + sizeof(TLockPolicy::Mutex) + sizeof(TLockPolicy::Mutex) + 4 bytes vptr [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: sizeof(ClockBase<TTime, TTick>) + sizeof(IRTCClock<TTime>) + 1 bytes known members + sizeof(TTick) + sizeof(TTick) + sizeof(TTick) + sizeof(TLockPolicy::Mutex) + sizeof(TLockPolicy::Mutex) + 4 bytes vptr [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
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
