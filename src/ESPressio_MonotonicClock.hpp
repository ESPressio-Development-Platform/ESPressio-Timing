#pragma once

#include "ESPressio_IClock.hpp"
#include "ESPressio_TimeSource.hpp"
#include "ESPressio_TimeTraits.hpp"

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
    /// The default source is resolved lazily on first clock read rather than during clock
    /// construction. This allows global/static PrecisionThread objects to be constructed
    /// before a platform installs its System providers without prematurely freezing the
    /// shared Timing source onto the portable fallback. On ESP32, once
    /// ESP32Platform::InstallSystemProviders() has installed the high-resolution counter
    /// provider, the first read creates the single shared GPTimer-backed Timing source.
    ///
    /// Every default PrecisionThread consumes this same monotonic clock/source; no hardware
    /// timer is allocated per thread.
    /// </remarks>
    template<
        typename TTime = DefaultClockTime,
        typename TTick = ClockTick
    >
    class MonotonicClock final :
        public virtual IClock<TTime> {
    private:
        ITimeSource* _explicitTimeSource = nullptr;

        ITimeSource* ResolveTimeSource() const noexcept {
            return _explicitTimeSource != nullptr
                ? _explicitTimeSource
                : HighResolutionTimeSource::GetInstance();
        }

        static TTime CreateTime(TTick nanoseconds, TTick resolution) {
            return TimeTraits<TTime>::template FromNanoseconds<TTick>(
                nanoseconds,
                resolution
            );
        }

    public:
        using TimeType = TTime;
        using TickType = TTick;

        /// <summary>
        /// Creates a monotonic clock over an optional explicit raw source. Null selects the
        /// process-wide shared HighResolutionTimeSource lazily on first read.
        /// </summary>
        explicit MonotonicClock(ITimeSource* timeSource = nullptr) noexcept
            : _explicitTimeSource(timeSource) {}

        /// <inheritdoc/>
        TTime GetTime() const override {
            auto* source = ResolveTimeSource();
            const auto ticksPerSecond = source->GetTicksPerSecond();
            TTick resolution = static_cast<TTick>(
                Internal::GetSourceResolution(ticksPerSecond)
            );
            if (resolution == 0) resolution = 1;
            const TTick nanoseconds = static_cast<TTick>(
                Internal::TicksToNanoseconds(
                    source->GetTicks(),
                    ticksPerSecond
                )
            );
            return CreateTime(nanoseconds, resolution);
        }

        /// <inheritdoc/>
        TTime GetResolution() const override {
            auto* source = ResolveTimeSource();
            TTick resolution = static_cast<TTick>(
                Internal::GetSourceResolution(source->GetTicksPerSecond())
            );
            if (resolution == 0) resolution = 1;
            return CreateTime(resolution, resolution);
        }

        /// <summary>Returns the raw source currently backing this clock.</summary>
        ITimeSource* GetTimeSource() const noexcept {
            return ResolveTimeSource();
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
