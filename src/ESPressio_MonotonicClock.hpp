#pragma once

#include "ESPressio_Clock.hpp"

namespace ESPressio {
namespace Timing {

    /// <summary>
    /// Process-wide monotonic clock facade over the shared high-resolution Timing source.
    /// </summary>
    /// <remarks>
    /// This clock deliberately has no settable or distributed-time semantics. It exposes
    /// elapsed physical time from <see cref="HighResolutionTimeSource"/> and is therefore
    /// suitable for scheduling, timeout measurement and other operations which must not
    /// move when <see cref="SystemClock"/> is stepped or slewed.
    ///
    /// On platforms which install a high-resolution counter provider, the shared
    /// HighResolutionTimeSource owns one provider counter. The ESP32 provider maps that
    /// counter to GPTimer, so every default PrecisionThread consumes the same GPTimer-backed
    /// monotonic timeline without allocating one hardware timer per thread.
    /// </remarks>
    template<
        typename TTime = DefaultClockTime,
        typename TTick = ClockTick
    >
    class MonotonicClock final :
        public ClockBase<TTime, TTick> {
    private:
        using Base = ClockBase<TTime, TTick>;

    public:
        using TimeType = TTime;
        using TickType = TTick;

        /// <summary>
        /// Creates a monotonic clock over the supplied raw source. A null source selects
        /// the process-wide shared HighResolutionTimeSource.
        /// </summary>
        explicit MonotonicClock(
            ITimeSource* timeSource = HighResolutionTimeSource::GetInstance()
        ) : Base(timeSource) {}

        /// <inheritdoc/>
        TTime GetTime() const override {
            const TTick sourceTime = this->GetSourceTime();
            TTick resolution = static_cast<TTick>(
                Internal::GetSourceResolution(
                    this->_timeSource->GetTicksPerSecond()
                )
            );
            if (resolution == 0) resolution = 1;
            return this->CreateTime(sourceTime, resolution);
        }

        /// <summary>
        /// Returns the process-wide shared monotonic clock for this public time representation.
        /// </summary>
        static MonotonicClock& GetInstance() {
            static MonotonicClock instance;
            return instance;
        }
    };

}
}
