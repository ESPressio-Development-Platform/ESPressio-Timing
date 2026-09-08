#pragma once

#include <cstdint>

#include "ESPressio_ClockTypes.hpp"

namespace ESPressio {

    namespace Timing {

        /// <summary>High-level acquisition state of a disciplined clock.</summary>
/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
enum
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
class ClockSynchronizationState : uint8_t {
            Unsynchronized,
            Acquiring,
            Synchronized
        };


        /// <summary>Controls whether synchronization corrects phase by slewing, startup stepping, or unconditional stepping.</summary>
/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
enum
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
class ClockSynchronizationAdjustmentMode : uint8_t {
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
/**
 * ESPressio Memory Audit
 * Members:
 * - LocalRequestTransmitTime (TTick): sizeof(TTick) [0 bytes dynamic allocation]
 * - RemoteRequestReceiveTime (TTick): sizeof(TTick) [0 bytes dynamic allocation]
 * - RemoteResponseTransmitTime (TTick): sizeof(TTick) [0 bytes dynamic allocation]
 * - LocalResponseReceiveTime (TTick): sizeof(TTick) [0 bytes dynamic allocation]
 * Total Memory: sizeof(TTick) + sizeof(TTick) + sizeof(TTick) + sizeof(TTick) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
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


        /// <summary>Canonical Timing-owned reason that a four-timestamp sample cannot enter clock discipline.</summary>
/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
enum
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
class ClockSynchronizationSampleRejectionReason : uint8_t {
            None = 0,
            InvalidTimestampOrder,
            RemoteProcessingExceedsLocalElapsed,
            RoundTripDelayExceeded
        };


        /// <summary>Transport-neutral validation evidence for one four-timestamp synchronization sample.</summary>
/**
 * ESPressio Memory Audit
 * Members:
 * - RejectionReason (ClockSynchronizationSampleRejectionReason): 1 bytes [0 bytes dynamic allocation]
 * - LocalElapsedNanoseconds (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - RemoteProcessingElapsedNanoseconds (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - RoundTripDelayNanoseconds (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * Total Memory: 28 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
struct ClockSynchronizationSampleValidation final {
            ClockSynchronizationSampleRejectionReason RejectionReason =
                ClockSynchronizationSampleRejectionReason::None;
            uint64_t LocalElapsedNanoseconds = 0;
            uint64_t RemoteProcessingElapsedNanoseconds = 0;
            uint64_t RoundTripDelayNanoseconds = 0;

            constexpr bool Accepted() const noexcept {
                return RejectionReason == ClockSynchronizationSampleRejectionReason::None;
            }
        };


        /// <summary>
        /// Applies Timing's transport-independent structural/delay admission rules without modifying discipline state.
        /// </summary>
        /// <remarks>
        /// This diagnostic surface exists so transports can retain reason-specific counters without duplicating Timing
        /// policy. ClockDiscipline remains authoritative for actual sample acceptance, filtering, slew and drift state.
        /// A zero maximumRoundTripDelayNanoseconds disables the delay limit, matching ClockSynchronizationConfig.
        /// </remarks>
        template<typename TTick = ClockTick>
        ClockSynchronizationSampleValidation ValidateClockSynchronizationSample(
            const ClockSynchronizationSample<TTick>& sample,
            uint64_t maximumRoundTripDelayNanoseconds
        ) noexcept {
            ClockSynchronizationSampleValidation result{};
            if (
                sample.LocalResponseReceiveTime < sample.LocalRequestTransmitTime ||
                sample.RemoteResponseTransmitTime < sample.RemoteRequestReceiveTime
            ) {
                result.RejectionReason =
                    ClockSynchronizationSampleRejectionReason::InvalidTimestampOrder;
                return result;
            }

            result.LocalElapsedNanoseconds = static_cast<uint64_t>(
                sample.LocalResponseReceiveTime - sample.LocalRequestTransmitTime);
            result.RemoteProcessingElapsedNanoseconds = static_cast<uint64_t>(
                sample.RemoteResponseTransmitTime - sample.RemoteRequestReceiveTime);

            if (result.RemoteProcessingElapsedNanoseconds > result.LocalElapsedNanoseconds) {
                result.RejectionReason =
                    ClockSynchronizationSampleRejectionReason::RemoteProcessingExceedsLocalElapsed;
                return result;
            }

            result.RoundTripDelayNanoseconds =
                result.LocalElapsedNanoseconds - result.RemoteProcessingElapsedNanoseconds;
            if (
                maximumRoundTripDelayNanoseconds > 0U &&
                result.RoundTripDelayNanoseconds > maximumRoundTripDelayNanoseconds
            ) {
                result.RejectionReason =
                    ClockSynchronizationSampleRejectionReason::RoundTripDelayExceeded;
            }
            return result;
        }


        /// <summary>Filtering, delay-rejection, slew, drift-learning, and synchronization-state configuration.</summary>
/**
 * ESPressio Memory Audit
 * Members:
 * - MaximumRoundTripDelayNanoseconds (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - MaximumSlewRatePpm (uint32_t): 4 bytes [0 bytes dynamic allocation]
 * - MaximumDriftCorrectionPpm (double): 8 bytes [0 bytes dynamic allocation]
 * - OffsetFilterWeight (double): 8 bytes [0 bytes dynamic allocation]
 * - ClockFilterWindowSamples (uint8_t): 1 bytes [0 bytes dynamic allocation]
 * - DriftFilterWeight (double): 8 bytes [0 bytes dynamic allocation]
 * - DriftLearningPhaseThresholdNanoseconds (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - MinimumDriftLearningIntervalNanoseconds (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - SynchronizationToleranceNanoseconds (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - MinimumSamplesForSynchronizedState (uint32_t): 4 bytes [0 bytes dynamic allocation]
 * - MaximumSampleAgeNanoseconds (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * Total Memory: 76 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
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
             * Number of recent valid exchanges retained by the NTP-style
             * minimum-delay clock filter. The lowest-delay observation is
             * least affected by variable transport/queue residence time.
             * Values are clamped to the fixed, allocation-free range 1..8.
             */
            uint8_t ClockFilterWindowSamples = 8;

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
/**
 * ESPressio Memory Audit
 * Members:
 * - Accepted (bool): 1 bytes [0 bytes dynamic allocation]
 * - MeasuredOffsetNanoseconds (int64_t): 8 bytes [0 bytes dynamic allocation]
 * - FilteredOffsetNanoseconds (int64_t): 8 bytes [0 bytes dynamic allocation]
 * - RoundTripDelayNanoseconds (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - EstimatedDriftPpm (double): 8 bytes [0 bytes dynamic allocation]
 * - AcceptedSampleCount (uint32_t): 4 bytes [0 bytes dynamic allocation]
 * - RejectedSampleCount (uint32_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 44 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
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
/**
 * ESPressio Memory Audit
 * Members:
 * - State (ClockSynchronizationState): 1 bytes [0 bytes dynamic allocation]
 * - LastMeasuredOffsetNanoseconds (int64_t): 8 bytes [0 bytes dynamic allocation]
 * - FilteredOffsetNanoseconds (int64_t): 8 bytes [0 bytes dynamic allocation]
 * - PendingPhaseCorrectionNanoseconds (int64_t): 8 bytes [0 bytes dynamic allocation]
 * - AppliedCorrectionNanoseconds (int64_t): 8 bytes [0 bytes dynamic allocation]
 * - LastRoundTripDelayNanoseconds (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - EstimatedDriftPpm (double): 8 bytes [0 bytes dynamic allocation]
 * - AcceptedSampleCount (uint32_t): 4 bytes [0 bytes dynamic allocation]
 * - RejectedSampleCount (uint32_t): 4 bytes [0 bytes dynamic allocation]
 * - LastAcceptedSampleLocalTime (TTick): sizeof(TTick) [0 bytes dynamic allocation]
 * - HasAcceptedSample (bool): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: 58 bytes known members + sizeof(TTick) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
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
