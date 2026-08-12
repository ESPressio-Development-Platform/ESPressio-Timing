#pragma once

#include "ESPressio_Clock.hpp"
#include "ESPressio_IStopwatchClock.hpp"

namespace ESPressio {

    namespace Timing {

        class StopwatchClock :
            public ClockBase,
            public IStopwatchClock {
            private:
                ClockTick _elapsedTime = 0;
                ClockTick _startTime = 0;
                bool _isRunning = false;

            public:
            // Constructor

                explicit StopwatchClock(
                    bool startImmediately = false,
                    ITimeSource* timeSource =
                        HighResolutionTimeSource::GetInstance()
                ) : ClockBase(timeSource) {
                    if (startImmediately) {
                        Start();
                    }
                }

            // Methods

                void Start() override {
                    if (_isRunning) {
                        return;
                    }

                    _startTime = GetSourceTime();
                    _isRunning = true;
                }

                void Stop() override {
                    if (!_isRunning) {
                        return;
                    }

                    _elapsedTime = GetNanoseconds(GetTime());
                    _isRunning = false;
                }

                void Reset() override {
                    _elapsedTime = 0;
                    _startTime = GetSourceTime();
                }

                void Restart() override {
                    _elapsedTime = 0;
                    _startTime = GetSourceTime();
                    _isRunning = true;
                }

            // Getters

                ClockTime GetTime() const override {
                    if (!_isRunning) {
                        return CreateClockTime(
                            _elapsedTime,
                            Internal::GetSourceResolution(
                                _timeSource->GetTicksPerSecond()
                            )
                        );
                    }

                    const ClockTick sourceTime = GetSourceTime();
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
                    return _isRunning;
                }

            // Setters

                void SetTime(ClockTime time) override {
                    _elapsedTime = GetNanoseconds(time);

                    if (_isRunning) {
                        _startTime = GetSourceTime();
                    }
                }
        };

    }

}
