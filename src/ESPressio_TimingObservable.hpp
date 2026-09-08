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
 * Inherited Memory Total: 4 bytes known bases + sizeof(std::enable_shared_from_this<ThreadSafeObservable>) + 4 bytes known members + sizeof(std::recursive_mutex) [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 4 bytes known bases + sizeof(std::enable_shared_from_this<ThreadSafeObservable>) + 4 bytes known members + sizeof(std::recursive_mutex) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
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
