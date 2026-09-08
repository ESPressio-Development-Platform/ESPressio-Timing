#pragma once

#include <memory>
#include <utility>

#include <ESPressio_Memory.hpp>
#include <ESPressio_ThreadSafeObservable.hpp>

namespace ESPressio {
namespace Timing {

/// <summary>Thread-safe observer dispatcher used by Timing implementations for typed lifecycle notifications.</summary>
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 20 bytes [ThreadSafeObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; ThreadSafeObservable: mutex_: native synchronization state may allocate platform resources lazily]
 * Members: none (standalone empty object occupies 1 byte; an eligible empty base may be optimized to 0 bytes).
 * Total Memory: 20 bytes [ThreadSafeObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; ThreadSafeObservable: mutex_: native synchronization state may allocate platform resources lazily]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class TimingObservable final :
    public Observable::ThreadSafeObservable {
public:
    /// <summary>Notifies observers implementing the requested Timing observer interface.</summary>
    /// <remarks>Exceptions raised by observers are isolated so they cannot alter Timing state.</remarks>
    template<typename TObserver, typename TCallback>
    void Notify(TCallback&& callback) {
        ExecuteNotification(
            [&](NotificationContext& notification) {
                notification.template WithObservers<TObserver>(
                    [&](TObserver* observer) {
                        try {
                            callback(observer);
                        } catch (...) {
                            /* Observer failures must not alter Timing state. */
                        }
                    }
                );
            }
        );
    }
};

/// <summary>Creates a shared Timing observer dispatcher in externally preferred memory.</summary>
/// <remarks>Timing observer bookkeeping is non-DMA state and therefore should not consume scarce internal RAM when an external-capable System provider is active.</remarks>
inline std::shared_ptr<TimingObservable>
CreateTimingObservable() {
    return System::Memory::MakeShared<
        TimingObservable,
        System::Memory::MemoryPolicy::ExternalPreferred
    >();
}

} // namespace Timing
} // namespace ESPressio
