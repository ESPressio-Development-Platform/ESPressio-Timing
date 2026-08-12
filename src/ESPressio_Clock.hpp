#pragma once

#include <limits>

#include "ESPressio_IClock.hpp"
#include "ESPressio_ITimeSource.hpp"
#include "ESPressio_TimeSource.hpp"

namespace ESPressio {

    namespace Timing {

        class ClockBase : public virtual IClock {
            protected:
                ITimeSource* _timeSource;

                explicit ClockBase(
                    ITimeSource* timeSource =
                        HighResolutionTimeSource::GetInstance()
                ) : _timeSource(
                        timeSource == nullptr
                            ? HighResolutionTimeSource::GetInstance()
                            : timeSource
                    ) { }

                ClockTick GetSourceTime() const {
                    return Internal::TicksToNanoseconds(
                        _timeSource->GetTicks(),
                        _timeSource->GetTicksPerSecond()
                    );
                }

                static ClockTick AddSaturated(
                    ClockTick left,
                    ClockTick right
                ) {
                    const ClockTick maximum =
                        std::numeric_limits<ClockTick>::max();

                    return right > maximum - left
                        ? maximum
                        : left + right;
                }

            public:
            // Getters

                static Units::UnitOrderOfMagnitude GetMagnitudeForResolution(
                    ClockTick resolution
                ) {
                    if (resolution >= NanosecondsPerSecond &&
                        resolution % NanosecondsPerSecond == 0) {
                        return Units::Base;
                    }

                    if (resolution >= NanosecondsPerMillisecond &&
                        resolution % NanosecondsPerMillisecond == 0) {
                        return Units::Milli;
                    }

                    if (resolution >= NanosecondsPerMicrosecond &&
                        resolution % NanosecondsPerMicrosecond == 0) {
                        return Units::Micro;
                    }

                    return Units::Nano;
                }

                static ClockTick GetMagnitudeScale(
                    Units::UnitOrderOfMagnitude magnitude
                ) {
                    switch (magnitude) {
                        case Units::Base: return NanosecondsPerSecond;
                        case Units::Milli: return NanosecondsPerMillisecond;
                        case Units::Micro: return NanosecondsPerMicrosecond;
                        default: return 1;
                    }
                }

                static ClockTime CreateClockTime(
                    ClockTick nanoseconds,
                    ClockTick resolution
                ) {
                    const Units::UnitOrderOfMagnitude magnitude =
                        GetMagnitudeForResolution(resolution);
                    return ClockTime(
                        nanoseconds / GetMagnitudeScale(magnitude),
                        magnitude
                    );
                }

                static ClockTick GetNanoseconds(const ClockTime& time) {
                    const int exponentDifference =
                        static_cast<int>(time.orderOfMagnitude) -
                        static_cast<int>(Units::Nano);
                    ClockTick value = time.value;

                    if (exponentDifference < 0) {
                        for (
                            int exponent = exponentDifference;
                            exponent < 0;
                            ++exponent
                        ) {
                            value /= 10;
                        }
                        return value;
                    }

                    const ClockTick maximum =
                        std::numeric_limits<ClockTick>::max();

                    for (
                        int exponent = 0;
                        exponent < exponentDifference;
                        ++exponent
                    ) {
                        if (value > maximum / 10) {
                            return maximum;
                        }
                        value *= 10;
                    }

                    return value;
                }

                ClockTime GetResolution() const override {
                    const ClockTick resolution =
                        Internal::GetSourceResolution(
                        _timeSource->GetTicksPerSecond()
                    );
                    return CreateClockTime(resolution, resolution);
                }

                ITimeSource* GetTimeSource() const {
                    return _timeSource;
                }
        };

        class ClockSettableBase :
            public ClockBase,
            public virtual IClockSettable {
            protected:
                ClockTick _baseTime = 0;
                ClockTick _baseSourceTime = 0;

                explicit ClockSettableBase(
                    ITimeSource* timeSource =
                        HighResolutionTimeSource::GetInstance()
                ) : ClockBase(timeSource),
                    _baseSourceTime(GetSourceTime()) { }

            public:
            // Getters

                ClockTime GetTime() const override {
                    const ClockTick sourceTime = GetSourceTime();
                    const ClockTick elapsed = sourceTime >= _baseSourceTime
                        ? sourceTime - _baseSourceTime
                        : 0;

                    return CreateClockTime(
                        AddSaturated(_baseTime, elapsed),
                        Internal::GetSourceResolution(
                            _timeSource->GetTicksPerSecond()
                        )
                    );
                }

            // Setters

                void SetTime(ClockTime time) override {
                    _baseSourceTime = GetSourceTime();
                    _baseTime = GetNanoseconds(time);
                }
        };

    }

}
