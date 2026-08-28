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
