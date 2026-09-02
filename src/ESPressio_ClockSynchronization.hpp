#pragma once

#include <cstdint>

#include "ESPressio_ClockTypes.hpp"

namespace ESPressio {

    namespace Timing {

        /// <summary>High-level acquisition state of a disciplined clock.</summary>
        enum class ClockSynchronizationState : uint8_t {
            Unsynchronized,
            Acquiring,
            Synchronized
        };


        /// <summary>Controls whether synchronization corrects phase by slewing, startup stepping, or unconditional stepping.</summary>
        enum class ClockSynchronizationAdjustmentMode : uint8_t {
            /*
             * Never move the public System Clock discontinuously.
             * Phase error is corrected by slewing.
             */
            SlewOnly,

            /*
             * Permit one immediate phase correction while the discipline is
             * not yet synchronized. Subsequent corrections slew.
             *
             * Intended for startup/bootstrap before deadline-driven work
             * begins.
             */
            StepIfUnsynchronized,

            /*
             * Always permit an immediate phase correction.
             *
             * This may move the System Clock forwards or backwards and should
             * not normally be used while monotonic consumers are active.
             */
            StepAlways
        };


        /// <summary>Four timestamps captured during a two-way clock synchronization exchange.</summary>
        /// <typeparam name="TTick">Raw timestamp type used by the synchronization transport.</typeparam>
        template<typename TTick = ClockTick>
        struct ClockSynchronizationSample {
            /*
             * Four timestamps from a two-way synchronization exchange:
             *
             * Local                         Remote
             *
             * T1  request transmit  -------->
             *                        <--------  T2 request receive
             *                        <--------  T3 response transmit
             * T4  response receive
             *
             * The transport is responsible only for capturing and carrying
             * these timestamps. Timing owns the offset/delay estimation.
             */
            TTick LocalRequestTransmitTime = 0;   // T1
            TTick RemoteRequestReceiveTime = 0;   // T2
            TTick RemoteResponseTransmitTime = 0; // T3
            TTick LocalResponseReceiveTime = 0;   // T4
        };


        /// <summary>Filtering, delay-rejection, slew, drift-learning, and synchronization-state configuration.</summary>
        struct ClockSynchronizationConfig {
            /*
             * Samples exceeding this measured network round-trip delay are
             * rejected. Zero disables the limit.
             */
            uint64_t MaximumRoundTripDelayNanoseconds =
                100000000ULL; // 100 ms

            /*
             * Maximum phase slew rate. 500 ppm means the disciplined clock
             * can gain/lose at most 500 ns per millisecond relative to the raw
             * source while removing phase error.
             */
            uint32_t MaximumSlewRatePpm = 500;

            /*
             * Maximum absolute learned rate correction.
             */
            double MaximumDriftCorrectionPpm = 2000.0;

            /*
             * Exponential filter weight for phase measurements.
             *
             * 1.0 = latest sample only.
             * 0.0 is normalized to 1.0.
             */
            double OffsetFilterWeight = 0.25;

            /*
             * Exponential filter weight for learned rate error.
             */
            double DriftFilterWeight = 0.10;

            /*
             * Drift is learned only after the phase servo has effectively
             * settled, preventing deliberate slewing from being mistaken for
             * oscillator drift.
             */
            uint64_t DriftLearningPhaseThresholdNanoseconds =
                1000000ULL; // 1 ms

            /*
             * Minimum elapsed local time between samples used for drift
             * estimation.
             */
            uint64_t MinimumDriftLearningIntervalNanoseconds =
                1000000000ULL; // 1 s

            /*
             * Phase threshold used for synchronized state.
             */
            uint64_t SynchronizationToleranceNanoseconds =
                1000000ULL; // 1 ms

            /*
             * Accepted samples required before synchronized state can be
             * reported.
             */
            uint32_t MinimumSamplesForSynchronizedState = 2;

            /*
             * A synchronized state becomes stale if no accepted sample arrives
             * within this interval. Zero disables staleness.
             */
            uint64_t MaximumSampleAgeNanoseconds =
                30000000000ULL; // 30 s
        };


        /// <summary>Outcome and current estimates produced when one synchronization sample is submitted.</summary>
        template<typename TTick = ClockTick>
        struct ClockSynchronizationResult {
            bool Accepted = false;

            int64_t MeasuredOffsetNanoseconds = 0;
            int64_t FilteredOffsetNanoseconds = 0;

            uint64_t RoundTripDelayNanoseconds = 0;

            double EstimatedDriftPpm = 0.0;

            uint32_t AcceptedSampleCount = 0;
            uint32_t RejectedSampleCount = 0;
        };


        /// <summary>Snapshot of synchronization state, phase correction, delay, drift, and sample counters.</summary>
        template<typename TTick = ClockTick>
        struct ClockSynchronizationStatus {
            ClockSynchronizationState State =
                ClockSynchronizationState::Unsynchronized;

            int64_t LastMeasuredOffsetNanoseconds = 0;
            int64_t FilteredOffsetNanoseconds = 0;

            int64_t PendingPhaseCorrectionNanoseconds = 0;
            int64_t AppliedCorrectionNanoseconds = 0;

            uint64_t LastRoundTripDelayNanoseconds = 0;

            double EstimatedDriftPpm = 0.0;

            uint32_t AcceptedSampleCount = 0;
            uint32_t RejectedSampleCount = 0;

            TTick LastAcceptedSampleLocalTime = 0;

            bool HasAcceptedSample = false;
        };

    }

}
