#pragma once

#include "ESPressio_Clock.hpp"
#include "ESPressio_IStopwatchClock.hpp"
#include "ESPressio_LockPolicy.hpp"
#include "ESPressio_ThreadSafeLockPolicy.hpp"

namespace ESPressio {

    namespace Timing {

        template <typename TLockPolicy>
        class BasicStopwatchClock :
            public ClockBase,
            public IStopwatchClock {
            protected:
                mutable typename TLockPolicy::Mutex _clockMutex;

            private:
                ClockTick _elapsedTime = 0;
                ClockTick _startTime = 0;
                bool _isRunning = false;

            public:
            // Constructor

                explicit BasicStopwatchClock(
                    bool startImmediately = false,
                    ITimeSource* timeSource =
                        BasicHighResolutionTimeSource<
                            TLockPolicy
                        >::GetInstance()
                ) : ClockBase(timeSource) {
                    if (startImmediately) {
                        Start();
                    }
                }

            // Methods

                void Start() override {
                    const ClockTick sourceTime = GetSourceTime();
                    typename TLockPolicy::Guard lock(_clockMutex);

                    if (_isRunning) {
                        return;
                    }

                    _startTime = sourceTime;
                    _isRunning = true;
                }

                void Stop() override {
                    const ClockTick sourceTime = GetSourceTime();
                    typename TLockPolicy::Guard lock(_clockMutex);

                    if (!_isRunning) {
                        return;
                    }

                    const ClockTick currentInterval =
                        sourceTime >= _startTime
                            ? sourceTime - _startTime
                            : 0;
                    _elapsedTime = AddSaturated(
                        _elapsedTime,
                        currentInterval
                    );
                    _isRunning = false;
                }

                void Reset() override {
                    const ClockTick sourceTime = GetSourceTime();
                    typename TLockPolicy::Guard lock(_clockMutex);
                    _elapsedTime = 0;
                    _startTime = sourceTime;
                }

                void Restart() override {
                    const ClockTick sourceTime = GetSourceTime();
                    typename TLockPolicy::Guard lock(_clockMutex);
                    _elapsedTime = 0;
                    _startTime = sourceTime;
                    _isRunning = true;
                }

            // Getters

                ClockTime GetTime() const override {
                    const ClockTick sourceTime = GetSourceTime();
                    typename TLockPolicy::Guard lock(_clockMutex);

                    if (!_isRunning) {
                        return CreateClockTime(
                            _elapsedTime,
                            Internal::GetSourceResolution(
                                _timeSource->GetTicksPerSecond()
                            )
                        );
                    }

                    const ClockTick currentInterval =
                        sourceTime >= _startTime
                            ? sourceTime - _startTime
                            : 0;

                    const ClockTick resolution =
                        Internal::GetSourceResolution(
                            _timeSource->GetTicksPerSecond()
                        );
                    return CreateClockTime(
                        AddSaturated(_elapsedTime, currentInterval),
                        resolution
                    );
                }

                ClockTime GetLapTime() const override {
                    return GetTime();
                }

                bool GetIsRunning() const override {
                    typename TLockPolicy::Guard lock(_clockMutex);
                    return _isRunning;
                }

            // Setters

                void SetTime(ClockTime time) override {
                    const ClockTick sourceTime = GetSourceTime();
                    typename TLockPolicy::Guard lock(_clockMutex);
                    _elapsedTime = GetNanoseconds(time);

                    if (_isRunning) {
                        _startTime = sourceTime;
                    }
                }
        };

        typedef BasicStopwatchClock<ThreadSafeLockPolicy> StopwatchClock;
        typedef BasicStopwatchClock<NoLockPolicy>
            SingleThreadedStopwatchClock;

    }

}
