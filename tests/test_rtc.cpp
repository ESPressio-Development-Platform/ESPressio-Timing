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
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
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
 * Inherited Memory Total: 24 bytes known/aligned storage + sizeof(TTick) + sizeof(TTick) + sizeof(TTick) [RTCClockBase: _observable: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 20 bytes; RTCClockBase: _observable: pointee: ThreadSafeObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; RTCClockBase: _observable: pointee: ThreadSafeObservable: mutex_: native synchronization state may allocate platform resources lazily]
 * Members:
 * - stored (DefaultClockTime): 12 bytes [0 bytes dynamic allocation]
 * Total Memory: 12 bytes known/aligned storage + 24 bytes known/aligned storage + sizeof(TTick) + sizeof(TTick) + sizeof(TTick) [RTCClockBase: _observable: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 20 bytes; RTCClockBase: _observable: pointee: ThreadSafeObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; RTCClockBase: _observable: pointee: ThreadSafeObservable: mutex_: native synchronization state may allocate platform resources lazily]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
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
