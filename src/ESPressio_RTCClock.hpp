#pragma once

#include "ESPressio_Clock.hpp"
#include "ESPressio_IRTCClock.hpp"
#include "ESPressio_LockPolicy.hpp"
#include "ESPressio_ThreadSafeLockPolicy.hpp"

namespace ESPressio {

    namespace Timing {

        template<
            typename TTime = DefaultClockTime,
            typename TLockPolicy =
                ThreadSafeLockPolicy,
            typename TTick = ClockTick
        >
        class RTCClockBase :
            public ClockBase<
                TTime,
                TTick
            >,
            public IRTCClock<
                TTime
            > {

            private:
                using Base =
                    ClockBase<
                        TTime,
                        TTick
                    >;

                TTick _rtcTime = 0;
                TTick _sourceTime = 0;
                TTick _rtcResolution;
                bool _isSynchronized = false;

                mutable
                    typename TLockPolicy::Mutex
                        _stateMutex;

                mutable
                    typename TLockPolicy::Mutex
                        _rtcIOMutex;


                void SetSynchronizedTimeInternal(
                    const TTime& time,
                    TTick sourceTime
                ) {
                    typename TLockPolicy::Guard
                        lock(_stateMutex);

                    _sourceTime =
                        sourceTime;

                    _rtcTime =
                        this->GetNanoseconds(
                            time
                        );

                    _isSynchronized =
                        true;
                }


            protected:
                explicit RTCClockBase(
                    const TTime& rtcResolution,
                    ITimeSource* timeSource =
                        HighResolutionTimeSourceT<
                            TLockPolicy
                        >::GetInstance()
                )
                    : Base(timeSource),
                      _rtcResolution(
                          this->GetNanoseconds(
                              rtcResolution
                          )
                      ) {
                }


                virtual bool ReadRTC(
                    TTime& time
                ) = 0;


                virtual bool WriteRTC(
                    const TTime& time
                ) = 0;


                void SetSynchronizedTime(
                    const TTime& time
                ) {
                    const TTick sourceTime =
                        this->GetSourceTime();

                    SetSynchronizedTimeInternal(
                        time,
                        sourceTime
                    );
                }


            public:
                using TimeType = TTime;
                using TickType = TTick;


                bool Synchronize() override {
                    TTime time;

                    typename TLockPolicy::Guard
                        ioLock(
                            _rtcIOMutex
                        );

                    if (!ReadRTC(time)) {
                        return false;
                    }

                    const TTick sourceTime =
                        this->GetSourceTime();

                    SetSynchronizedTimeInternal(
                        time,
                        sourceTime
                    );

                    return true;
                }


                void OnRTCInterrupt() override {
                    Synchronize();
                }


                void OnRTCInterrupt(
                    const TTime& time
                ) override {
                    const TTick sourceTime =
                        this->GetSourceTime();

                    SetSynchronizedTimeInternal(
                        time,
                        sourceTime
                    );
                }


                bool TrySetTime(
                    const TTime& time
                ) {
                    typename TLockPolicy::Guard
                        ioLock(
                            _rtcIOMutex
                        );

                    if (!WriteRTC(time)) {
                        return false;
                    }

                    const TTick sourceTime =
                        this->GetSourceTime();

                    SetSynchronizedTimeInternal(
                        time,
                        sourceTime
                    );

                    return true;
                }


                TTime GetTime() const override {
                    const TTick sourceTime =
                        this->GetSourceTime();

                    typename TLockPolicy::Guard
                        lock(
                            _stateMutex
                        );

                    if (!_isSynchronized) {
                        return
                            this->CreateTime(
                                0,
                                _rtcResolution
                            );
                    }

                    const TTick elapsed =
                        sourceTime >=
                            _sourceTime
                            ? sourceTime -
                                _sourceTime
                            : 0;

                    return
                        this->CreateTime(
                            this->AddSaturated(
                                _rtcTime,
                                elapsed
                            ),
                            _rtcResolution
                        );
                }


                TTime GetResolution() const override {
                    const TTick
                        sourceResolution =
                            static_cast<TTick>(
                                Internal::
                                    GetSourceResolution(
                                        this->_timeSource->
                                            GetTicksPerSecond()
                                    )
                            );

                    const TTick resolution =
                        _rtcResolution >
                            sourceResolution
                            ? _rtcResolution
                            : sourceResolution;

                    return
                        this->CreateTime(
                            resolution,
                            resolution
                        );
                }


                bool GetIsSynchronized() const override {
                    typename TLockPolicy::Guard
                        lock(
                            _stateMutex
                        );

                    return _isSynchronized;
                }


                void SetTime(
                    const TTime& time
                ) override {
                    TrySetTime(
                        time
                    );
                }
        };


        template<
            typename TTime = DefaultClockTime,
            typename TTick = ClockTick
        >
        using SingleThreadedRTCClockBase =
            RTCClockBase<
                TTime,
                NoLockPolicy,
                TTick
            >;

    }

}
