#pragma once

#include <mutex>

#include "ESPressio_Clock.hpp"
#include "ESPressio_IRTCClock.hpp"

namespace ESPressio {

    namespace Timing {

        /*
            Derive from `RTCClockBase` to bind an external or integrated RTC.
            The derived type performs device I/O in ReadRTC/WriteRTC. An RTC
            interrupt can capture an exact timestamp and defer a call to
            OnRTCInterrupt(time), avoiding bus access in interrupt context.
        */
        class RTCClockBase :
            public ClockBase,
            public IRTCClock {
            private:
                ClockTick _rtcTime = 0;
                ClockTick _sourceTime = 0;
                ClockTick _rtcResolution;
                bool _isSynchronized = false;
                mutable std::mutex _stateMutex;
                mutable std::mutex _rtcIOMutex;

                void _setSynchronizedTime(
                    ClockTime time,
                    ClockTick sourceTime
                ) {
                    std::lock_guard<std::mutex> lock(_stateMutex);
                    _sourceTime = sourceTime;
                    _rtcTime = GetNanoseconds(time);
                    _isSynchronized = true;
                }

            protected:
                explicit RTCClockBase(
                    ClockTime rtcResolution,
                    ITimeSource* timeSource =
                        HighResolutionTimeSource::GetInstance()
                ) : ClockBase(timeSource),
                    _rtcResolution(GetNanoseconds(rtcResolution)) { }

                virtual bool ReadRTC(ClockTime& time) = 0;
                virtual bool WriteRTC(ClockTime time) = 0;

                void SetSynchronizedTime(ClockTime time) {
                    const ClockTick sourceTime = GetSourceTime();
                    _setSynchronizedTime(time, sourceTime);
                }

            public:
            // Methods

                bool Synchronize() override {
                    ClockTime time;

                    std::lock_guard<std::mutex> ioLock(_rtcIOMutex);

                    if (!ReadRTC(time)) {
                        return false;
                    }

                    const ClockTick sourceTime = GetSourceTime();
                    _setSynchronizedTime(time, sourceTime);
                    return true;
                }

                void OnRTCInterrupt() override {
                    Synchronize();
                }

                void OnRTCInterrupt(ClockTime time) override {
                    const ClockTick sourceTime = GetSourceTime();
                    _setSynchronizedTime(time, sourceTime);
                }

                bool TrySetTime(ClockTime time) {
                    std::lock_guard<std::mutex> ioLock(_rtcIOMutex);

                    if (!WriteRTC(time)) {
                        return false;
                    }

                    const ClockTick sourceTime = GetSourceTime();
                    _setSynchronizedTime(time, sourceTime);
                    return true;
                }

            // Getters

                ClockTime GetTime() const override {
                    const ClockTick sourceTime = GetSourceTime();
                    std::lock_guard<std::mutex> lock(_stateMutex);

                    if (!_isSynchronized) {
                        return CreateClockTime(0, _rtcResolution);
                    }

                    const ClockTick elapsed = sourceTime >= _sourceTime
                        ? sourceTime - _sourceTime
                        : 0;

                    return CreateClockTime(
                        AddSaturated(_rtcTime, elapsed),
                        _rtcResolution
                    );
                }

                ClockTime GetResolution() const override {
                    const ClockTick sourceResolution =
                        Internal::GetSourceResolution(
                            _timeSource->GetTicksPerSecond()
                        );

                    const ClockTick resolution =
                        _rtcResolution > sourceResolution
                        ? _rtcResolution
                        : sourceResolution;
                    return CreateClockTime(resolution, resolution);
                }

                bool GetIsSynchronized() const override {
                    std::lock_guard<std::mutex> lock(_stateMutex);
                    return _isSynchronized;
                }

            // Setters

                void SetTime(ClockTime time) override {
                    TrySetTime(time);
                }
        };

    }

}
