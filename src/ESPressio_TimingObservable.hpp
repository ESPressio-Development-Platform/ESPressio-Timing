#pragma once

#include <memory>
#include <utility>

#include <ESPressio_ThreadSafeObservable.hpp>

namespace ESPressio {
namespace Timing {

class TimingObservable final :
    public Observable::ThreadSafeObservable {
public:
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

inline std::shared_ptr<TimingObservable>
CreateTimingObservable() {
    return std::make_shared<TimingObservable>();
}

} // namespace Timing
} // namespace ESPressio
