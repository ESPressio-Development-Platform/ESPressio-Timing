#pragma once

#include "ESPressio_Clock.hpp"
#include "ESPressio_IRTCClock.hpp"
#include "ESPressio_LockPolicy.hpp"
#include "ESPressio_ThreadSafeLockPolicy.hpp"

namespace ESPressio {

    namespace Timing {

        /*
            Derive from `BasicRTCClockBase` to bind an external or integrated
            RTC. The derived type performs device I/O in ReadRTC/WriteRTC. An
            RTC interrupt can capture an exact timestamp and defer a call to
            OnRTCInterrupt(time), avoiding bus access in interrupt context.
        */
        template <typename TLockPolicy>
        class BasicRTCClockBase :
            public ClockBase,
            public IRTCClock {
            private:
                ClockTick _rtcTime = 0;
                ClockTick _sourceTime = 0;
                ClockTick _rtcResolution;
                bool _isSynchronized = false;
                mutable typename TLockPolicy::Mutex _stateMutex;
                mutable typename TLockPolicy::Mutex _rtcIOMutex;

                void _setSynchronizedTime(
                    ClockTime time,
                    ClockTick sourceTime
                ) {
                    typename TLockPolicy::Guard lock(_stateMutex);
                    _sourceTime = sourceTime;
                    _rtcTime = ClockBase::GetNanoseconds(time);
                    _isSynchronized = true;
                }

            protected:
                explicit BasicRTCClockBase(
                    ClockTime rtcResolution,
                    ITimeSource* timeSource =
                        BasicHighResolutionTimeSource<
                            TLockPolicy
                        >::GetInstance()
                ) : ClockBase(timeSource),
                    _rtcResolution(
                        ClockBase::GetNanoseconds(rtcResolution)
                    ) { }

                virtual bool ReadRTC(ClockTime& time) = 0;
                virtual bool WriteRTC(ClockTime time) = 0;

                void SetSynchronizedTime(ClockTime time) {
                    const ClockTick sourceTime = this->GetSourceTime();
                    _setSynchronizedTime(time, sourceTime);
                }

            public:
            // Methods

                bool Synchronize() override {
                    ClockTime time;
                    typename TLockPolicy::Guard ioLock(_rtcIOMutex);

                    if (!ReadRTC(time)) {
                        return false;
                    }

                    const ClockTick sourceTime = this->GetSourceTime();
                    _setSynchronizedTime(time, sourceTime);
                    return true;
                }

                void OnRTCInterrupt() override {
                    Synchronize();
                }

                void OnRTCInterrupt(ClockTime time) override {
                    const ClockTick sourceTime = this->GetSourceTime();
                    _setSynchronizedTime(time, sourceTime);
                }

                bool TrySetTime(ClockTime time) {
                    typename TLockPolicy::Guard ioLock(_rtcIOMutex);

                    if (!WriteRTC(time)) {
                        return false;
                    }

                    const ClockTick sourceTime = this->GetSourceTime();
                    _setSynchronizedTime(time, sourceTime);
                    return true;
                }

            // Getters

                ClockTime GetTime() const override {
                    const ClockTick sourceTime = this->GetSourceTime();
                    typename TLockPolicy::Guard lock(_stateMutex);

                    if (!_isSynchronized) {
                        return ClockBase::CreateClockTime(
                            0,
                            _rtcResolution
                        );
                    }

                    const ClockTick elapsed = sourceTime >= _sourceTime
                        ? sourceTime - _sourceTime
                        : 0;

                    return ClockBase::CreateClockTime(
                        ClockBase::AddSaturated(_rtcTime, elapsed),
                        _rtcResolution
                    );
                }

                ClockTime GetResolution() const override {
                    const ClockTick sourceResolution =
                        Internal::GetSourceResolution(
                            this->_timeSource->GetTicksPerSecond()
                        );

                    const ClockTick resolution =
                        _rtcResolution > sourceResolution
                        ? _rtcResolution
                        : sourceResolution;
                    return ClockBase::CreateClockTime(
                        resolution,
                        resolution
                    );
                }

                bool GetIsSynchronized() const override {
                    typename TLockPolicy::Guard lock(_stateMutex);
                    return _isSynchronized;
                }

            // Setters

                void SetTime(ClockTime time) override {
                    TrySetTime(time);
                }
        };

        typedef BasicRTCClockBase<ThreadSafeLockPolicy> RTCClockBase;
        typedef BasicRTCClockBase<NoLockPolicy> SingleThreadedRTCClockBase;

    }

}
