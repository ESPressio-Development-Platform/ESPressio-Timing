#pragma once

#include <memory>
#include <utility>

#include <ESPressio_Memory.hpp>
#include <ESPressio_ThreadSafeObservable.hpp>

namespace ESPressio {
namespace Timing {

/// <summary>Thread-safe observer dispatcher used by Timing implementations for typed lifecycle notifications.</summary>
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
