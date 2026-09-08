#pragma once

#include <cstdint>

#include <ESPressio_Platform.hpp>

#include "ESPressio_GPTimerTimeSource.hpp"
#include "ESPressio_IStopwatchClock.hpp"
#include "ESPressio_StopwatchClock.hpp"

#if ESPRESSIO_TIMING_HAS_GPTIMER

namespace ESPressio {
namespace Timing {

    /// <summary>Stopwatch clock backed directly by the platform GPTimer time source.</summary>
    /// <remarks>Availability and initialization status are exposed explicitly so applications can detect unsupported or failed GPTimer initialization.</remarks>
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members:
 * - _timeSource (GPTimerTimeSource): 16 bytes [_counter: owned object: 4 bytes]
 * - _stopwatch (StopwatchClock<TTime, TLockPolicy, TTick>): 24 bytes known/aligned storage + sizeof(TTick) + sizeof(TTick) [_observable: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 20 bytes; _observable: pointee: ThreadSafeObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; _observable: pointee: ThreadSafeObservable: mutex_: native synchronization state may allocate platform resources lazily]
 * Total Memory: 20 bytes known/aligned storage + 24 bytes known/aligned storage + sizeof(TTick) + sizeof(TTick) [_timeSource: _counter: owned object: 4 bytes; _stopwatch: _observable: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 20 bytes; _stopwatch: _observable: pointee: ThreadSafeObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; _stopwatch: _observable: pointee: ThreadSafeObservable: mutex_: native synchronization state may allocate platform resources lazily]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
template<
        typename TTime = DefaultClockTime,
        typename TLockPolicy = ThreadSafeLockPolicy,
        typename TTick = ClockTick
    >
    class GPTimerClock : public IStopwatchClock<TTime> {
    private:
        GPTimerTimeSource _timeSource;
        StopwatchClock<TTime, TLockPolicy, TTick> _stopwatch;

    public:
        using TimeType = TTime;
        using TickType = TTick;

        /// <summary>Creates a GPTimer-backed stopwatch at the requested timer resolution.</summary>
        explicit GPTimerClock(
            bool startImmediately = false,
            uint32_t requestedResolution =
                ESPRESSIO_TIMING_GPTIMER_DEFAULT_RESOLUTION_HZ
        )
            : _timeSource(requestedResolution),
              _stopwatch(
                  startImmediately && _timeSource.GetIsAvailable(),
                  &_timeSource
              ) {}

        GPTimerClock(const GPTimerClock&) = delete;
        GPTimerClock& operator=(const GPTimerClock&) = delete;
        GPTimerClock(GPTimerClock&&) = delete;
        GPTimerClock& operator=(GPTimerClock&&) = delete;

        /// <inheritdoc/>
        void Start() override {
            if (_timeSource.GetIsAvailable()) _stopwatch.Start();
        }

        /// <inheritdoc/>
        void Stop() override { _stopwatch.Stop(); }
        /// <inheritdoc/>
        void Reset() override { _stopwatch.Reset(); }

        /// <inheritdoc/>
        void Restart() override {
            if (_timeSource.GetIsAvailable()) _stopwatch.Restart();
        }

        /// <inheritdoc/>
        TTime GetTime() const override { return _stopwatch.GetTime(); }
        /// <inheritdoc/>
        TTime GetResolution() const override { return _stopwatch.GetResolution(); }
        /// <inheritdoc/>
        TTime GetLapTime() const override { return _stopwatch.GetLapTime(); }

        /// <inheritdoc/>
        bool GetIsRunning() const override {
            return _timeSource.GetIsAvailable() && _stopwatch.GetIsRunning();
        }

        /// <summary>Indicates whether the underlying GPTimer source initialized successfully.</summary>
        bool GetIsAvailable() const { return _timeSource.GetIsAvailable(); }

        /// <summary>Returns the platform result produced while initializing the GPTimer source.</summary>
        System::PlatformResult GetInitializationResult() const {
            return _timeSource.GetInitializationResult();
        }

        /// <summary>Registers an observer for stopwatch lifecycle notifications.</summary>
        Observable::ObserverHandlePtr RegisterObserver(
            IStopwatchClockObserver<TTime, TTick>* observer
        ) {
            return _stopwatch.RegisterObserver(observer);
        }

        /// <summary>Unregisters a stopwatch lifecycle observer.</summary>
        void UnregisterObserver(
            IStopwatchClockObserver<TTime, TTick>* observer
        ) {
            _stopwatch.UnregisterObserver(observer);
        }

        /// <summary>Returns the GPTimer time source used by this clock.</summary>
        ITimeSource* GetTimeSource() { return &_timeSource; }

        /// <inheritdoc/>
        void SetTime(const TTime& time) override {
            _stopwatch.SetTime(time);
        }
    };

    /// <summary>GPTimer clock variant using a no-op lock policy for single-threaded consumers.</summary>
    template<typename TTime = DefaultClockTime, typename TTick = ClockTick>
    using SingleThreadedGPTimerClock = GPTimerClock<TTime, NoLockPolicy, TTick>;

}
}

#endif
