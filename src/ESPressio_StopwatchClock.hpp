#pragma once

#include "ESPressio_Clock.hpp"
#include "ESPressio_IStopwatchClock.hpp"
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
        class StopwatchClock :
            public ClockBase<
                TTime,
                TTick
            >,
            public IStopwatchClock<
                TTime
            > {

            protected:
                using Base =
                    ClockBase<
                        TTime,
                        TTick
                    >;

                mutable
                    typename TLockPolicy::Mutex
                        _clockMutex;

            private:
                TTick _elapsedTime = 0;
                TTick _startTime = 0;
                bool _isRunning = false;


            public:
                using TimeType = TTime;
                using TickType = TTick;


                explicit StopwatchClock(
                    bool startImmediately = false,
                    ITimeSource* timeSource =
                        HighResolutionTimeSourceT<
                            TLockPolicy
                        >::GetInstance()
                )
                    : Base(timeSource) {

                    if (startImmediately) {
                        Start();
                    }
                }


                void Start() override {
                    const TTick sourceTime =
                        this->GetSourceTime();

                    typename TLockPolicy::Guard
                        lock(_clockMutex);

                    if (_isRunning) {
                        return;
                    }

                    _startTime = sourceTime;
                    _isRunning = true;
                }


                void Stop() override {
                    const TTick sourceTime =
                        this->GetSourceTime();

                    typename TLockPolicy::Guard
                        lock(_clockMutex);

                    if (!_isRunning) {
                        return;
                    }

                    const TTick currentInterval =
                        sourceTime >= _startTime
                            ? sourceTime -
                                _startTime
                            : 0;

                    _elapsedTime =
                        this->AddSaturated(
                            _elapsedTime,
                            currentInterval
                        );

                    _isRunning = false;
                }


                void Reset() override {
                    const TTick sourceTime =
                        this->GetSourceTime();

                    typename TLockPolicy::Guard
                        lock(_clockMutex);

                    _elapsedTime = 0;
                    _startTime = sourceTime;
                }


                void Restart() override {
                    const TTick sourceTime =
                        this->GetSourceTime();

                    typename TLockPolicy::Guard
                        lock(_clockMutex);

                    _elapsedTime = 0;
                    _startTime = sourceTime;
                    _isRunning = true;
                }


                TTime GetTime() const override {
                    const TTick sourceTime =
                        this->GetSourceTime();

                    typename TLockPolicy::Guard
                        lock(_clockMutex);

                    const TTick resolution =
                        static_cast<TTick>(
                            Internal::
                                GetSourceResolution(
                                    this->_timeSource->
                                        GetTicksPerSecond()
                                )
                        );

                    if (!_isRunning) {
                        return
                            this->CreateTime(
                                _elapsedTime,
                                resolution
                            );
                    }

                    const TTick currentInterval =
                        sourceTime >= _startTime
                            ? sourceTime -
                                _startTime
                            : 0;

                    return
                        this->CreateTime(
                            this->AddSaturated(
                                _elapsedTime,
                                currentInterval
                            ),
                            resolution
                        );
                }


                TTime GetLapTime() const override {
                    return GetTime();
                }


                bool GetIsRunning() const override {
                    typename TLockPolicy::Guard
                        lock(_clockMutex);

                    return _isRunning;
                }


                void SetTime(
                    const TTime& time
                ) override {
                    const TTick sourceTime =
                        this->GetSourceTime();

                    typename TLockPolicy::Guard
                        lock(_clockMutex);

                    _elapsedTime =
                        this->GetNanoseconds(
                            time
                        );

                    if (_isRunning) {
                        _startTime =
                            sourceTime;
                    }
                }
        };


        template<
            typename TTime = DefaultClockTime,
            typename TTick = ClockTick
        >
        using SingleThreadedStopwatchClock =
            StopwatchClock<
                TTime,
                NoLockPolicy,
                TTick
            >;

    }

}
