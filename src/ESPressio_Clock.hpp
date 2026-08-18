#pragma once

#include <limits>

#include "ESPressio_IClock.hpp"
#include "ESPressio_ITimeSource.hpp"
#include "ESPressio_TimeSource.hpp"
#include "ESPressio_TimeTraits.hpp"
#include "ESPressio_LockPolicy.hpp"
#include "ESPressio_ThreadSafeLockPolicy.hpp"

namespace ESPressio {

    namespace Timing {

        template<
            typename TTime = DefaultClockTime,
            typename TTick = ClockTick
        >
        class ClockBase :
            public virtual IClock<TTime> {

            protected:
                ITimeSource* _timeSource;

                explicit ClockBase(
                    ITimeSource* timeSource =
                        HighResolutionTimeSource::
                            GetInstance()
                )
                    : _timeSource(
                        timeSource == nullptr
                            ? HighResolutionTimeSource::
                                GetInstance()
                            : timeSource
                    ) {
                }


                TTick GetSourceTime() const {
                    return static_cast<TTick>(
                        Internal::TicksToNanoseconds(
                            _timeSource->GetTicks(),
                            _timeSource->
                                GetTicksPerSecond()
                        )
                    );
                }


                static TTick AddSaturated(
                    TTick left,
                    TTick right
                ) {
                    const TTick maximum =
                        std::numeric_limits<
                            TTick
                        >::max();

                    return
                        right >
                            maximum -
                            left
                            ? maximum
                            : left +
                                right;
                }


                static TTime CreateTime(
                    TTick nanoseconds,
                    TTick resolution
                ) {
                    return
                        TimeTraits<TTime>::
                            template
                            FromNanoseconds<TTick>(
                                nanoseconds,
                                resolution
                            );
                }


                static TTick GetNanoseconds(
                    const TTime& time
                ) {
                    return
                        TimeTraits<TTime>::
                            template
                            ToNanoseconds<TTick>(
                                time
                            );
                }


            public:
                using TimeType = TTime;
                using TickType = TTick;


                TTime GetResolution() const override {
                    const TTick resolution =
                        static_cast<TTick>(
                            Internal::
                                GetSourceResolution(
                                    _timeSource->
                                        GetTicksPerSecond()
                                )
                        );

                    return CreateTime(
                        resolution,
                        resolution
                    );
                }


                ITimeSource*
                GetTimeSource() const {
                    return _timeSource;
                }
        };


        template<
            typename TTime = DefaultClockTime,
            typename TLockPolicy =
                ThreadSafeLockPolicy,
            typename TTick = ClockTick
        >
        class ClockSettableBase :
            public ClockBase<
                TTime,
                TTick
            >,
            public virtual IClockSettable<
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

                TTick _baseTime = 0;
                TTick _baseSourceTime = 0;


                explicit ClockSettableBase(
                    ITimeSource* timeSource =
                        HighResolutionTimeSourceT<
                            TLockPolicy
                        >::GetInstance()
                )
                    : Base(timeSource),
                      _baseSourceTime(
                          this->GetSourceTime()
                      ) {
                }


            public:
                using TimeType = TTime;
                using TickType = TTick;


                TTime GetTime() const override {
                    const TTick sourceTime =
                        this->GetSourceTime();

                    typename TLockPolicy::Guard
                        lock(_clockMutex);

                    const TTick elapsed =
                        sourceTime >=
                            _baseSourceTime
                            ? sourceTime -
                                _baseSourceTime
                            : 0;

                    return this->CreateTime(
                        this->AddSaturated(
                            _baseTime,
                            elapsed
                        ),
                        static_cast<TTick>(
                            Internal::
                                GetSourceResolution(
                                    this->_timeSource->
                                        GetTicksPerSecond()
                                )
                        )
                    );
                }


                void SetTime(
                    const TTime& time
                ) override {
                    const TTick sourceTime =
                        this->GetSourceTime();

                    typename TLockPolicy::Guard
                        lock(_clockMutex);

                    _baseSourceTime =
                        sourceTime;

                    _baseTime =
                        this->GetNanoseconds(
                            time
                        );
                }
        };

    }

}
