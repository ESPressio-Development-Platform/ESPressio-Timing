#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

#include "ESPressio_ClockSynchronization.hpp"

namespace ESPressio {

    namespace Timing {

        namespace Internal {

            inline int64_t SaturatingSignedDifference(
                uint64_t left,
                uint64_t right
            ) {
                if (left >= right) {
                    const uint64_t difference =
                        left - right;

                    return
                        difference >
                            static_cast<uint64_t>(
                                std::numeric_limits<int64_t>::max()
                            )
                            ? std::numeric_limits<int64_t>::max()
                            : static_cast<int64_t>(
                                difference
                            );
                }

                const uint64_t difference =
                    right - left;

                if (
                    difference >
                    static_cast<uint64_t>(
                        std::numeric_limits<int64_t>::max()
                    )
                ) {
                    return
                        std::numeric_limits<int64_t>::min();
                }

                return
                    -static_cast<int64_t>(
                        difference
                    );
            }


            inline int64_t SaturatingSignedAdd(
                int64_t left,
                int64_t right
            ) {
                if (
                    right > 0 &&
                    left >
                        std::numeric_limits<int64_t>::max() -
                        right
                ) {
                    return
                        std::numeric_limits<int64_t>::max();
                }

                if (
                    right < 0 &&
                    left <
                        std::numeric_limits<int64_t>::min() -
                        right
                ) {
                    return
                        std::numeric_limits<int64_t>::min();
                }

                return left + right;
            }


            inline int64_t SaturatingSignedSubtract(
                int64_t left,
                int64_t right
            ) {
                if (
                    right > 0 &&
                    left <
                        std::numeric_limits<int64_t>::min() +
                        right
                ) {
                    return
                        std::numeric_limits<int64_t>::min();
                }

                if (
                    right < 0 &&
                    left >
                        std::numeric_limits<int64_t>::max() +
                        right
                ) {
                    return
                        std::numeric_limits<int64_t>::max();
                }

                return left - right;
            }


            inline uint64_t AbsoluteSignedValue(
                int64_t value
            ) {
                if (value >= 0) {
                    return static_cast<uint64_t>(value);
                }

                if (
                    value ==
                    std::numeric_limits<int64_t>::min()
                ) {
                    return
                        static_cast<uint64_t>(
                            std::numeric_limits<int64_t>::max()
                        ) +
                        1ULL;
                }

                return
                    static_cast<uint64_t>(
                        -value
                    );
            }


            template<typename TTick>
            inline TTick ApplySignedTickCorrection(
                TTick value,
                int64_t correction
            ) {
                if (correction >= 0) {
                    const uint64_t positive =
                        static_cast<uint64_t>(
                            correction
                        );

                    return
                        positive >
                            static_cast<uint64_t>(
                                std::numeric_limits<TTick>::max() -
                                value
                            )
                            ? std::numeric_limits<TTick>::max()
                            : static_cast<TTick>(
                                value +
                                static_cast<TTick>(
                                    positive
                                )
                              );
                }

                const uint64_t magnitude =
                    correction ==
                        std::numeric_limits<int64_t>::min()
                        ? static_cast<uint64_t>(
                            std::numeric_limits<int64_t>::max()
                          ) +
                          1ULL
                        : static_cast<uint64_t>(
                            -correction
                          );

                return
                    magnitude >
                        static_cast<uint64_t>(
                            value
                        )
                        ? 0
                        : static_cast<TTick>(
                            value -
                            static_cast<TTick>(
                                magnitude
                            )
                          );
            }


            inline double NormalizeFilterWeight(
                double value
            ) {
                if (value <= 0.0) {
                    return 1.0;
                }

                return
                    value > 1.0
                        ? 1.0
                        : value;
            }


            inline double ClampDriftPpm(
                double value,
                double maximumAbsolute
            ) {
                if (maximumAbsolute <= 0.0) {
                    return 0.0;
                }

                return
                    std::max(
                        -maximumAbsolute,
                        std::min(
                            maximumAbsolute,
                            value
                        )
                    );
            }


            inline uint32_t ClampSlewRatePpm(
                uint32_t value
            ) {
                /*
                 * Keep the maximum phase rate far below 1,000,000 ppm so a
                 * negative slew can never make a normally progressing raw
                 * clock run backwards.
                 */
                return
                    std::min<uint32_t>(
                        value,
                        100000U
                    );
            }

        }


        /*
         * Transport-independent clock discipline.
         *
         * It performs:
         *   - NTP-style four-timestamp offset/delay calculation;
         *   - malformed/high-delay sample rejection;
         *   - filtered phase estimation;
         *   - conservative residual drift estimation;
         *   - monotonic phase slewing;
         *   - continuous learned rate correction.
         *
         * No network/radio concepts are present here.
         */
        template<typename TTick = ClockTick>
        class ClockDiscipline {
            private:
                ClockSynchronizationConfig
                    _config;

                bool _hasAcceptedSample = false;
                bool _hasFilteredOffset = false;

                int64_t _lastMeasuredOffsetNanoseconds = 0;
                int64_t _filteredOffsetNanoseconds = 0;
                uint64_t _lastRoundTripDelayNanoseconds = 0;

                uint32_t _acceptedSampleCount = 0;
                uint32_t _rejectedSampleCount = 0;

                TTick _lastAcceptedSampleLocalTime = 0;

                bool _hasPreviousDriftSample = false;
                int64_t _previousDriftOffsetNanoseconds = 0;
                TTick _previousDriftSampleLocalTime = 0;

                double _estimatedDriftPpm = 0.0;

                int64_t _pendingPhaseCorrectionNanoseconds = 0;
                int64_t _appliedCorrectionNanoseconds = 0;

                bool _advanceInitialized = false;
                TTick _lastAdvanceRawTime = 0;

                double _phaseFractionNanoseconds = 0.0;
                double _frequencyFractionNanoseconds = 0.0;


                static bool CalculateSample(
                    const ClockSynchronizationSample<TTick>& sample,
                    int64_t& offsetNanoseconds,
                    uint64_t& roundTripDelayNanoseconds
                ) {
                    if (
                        sample.LocalResponseReceiveTime <
                            sample.LocalRequestTransmitTime ||
                        sample.RemoteResponseTransmitTime <
                            sample.RemoteRequestReceiveTime
                    ) {
                        return false;
                    }

                    const uint64_t localElapsed =
                        static_cast<uint64_t>(
                            sample.LocalResponseReceiveTime -
                            sample.LocalRequestTransmitTime
                        );

                    const uint64_t remoteProcessingElapsed =
                        static_cast<uint64_t>(
                            sample.RemoteResponseTransmitTime -
                            sample.RemoteRequestReceiveTime
                        );

                    if (
                        remoteProcessingElapsed >
                        localElapsed
                    ) {
                        /*
                         * A negative network round trip is not physically
                         * meaningful and usually indicates malformed,
                         * reordered, or mixed-exchange timestamps.
                         */
                        return false;
                    }

                    roundTripDelayNanoseconds =
                        localElapsed -
                        remoteProcessingElapsed;

                    const int64_t remoteReceiveMinusLocalSend =
                        Internal::SaturatingSignedDifference(
                            static_cast<uint64_t>(
                                sample.RemoteRequestReceiveTime
                            ),
                            static_cast<uint64_t>(
                                sample.LocalRequestTransmitTime
                            )
                        );

                    const int64_t remoteSendMinusLocalReceive =
                        Internal::SaturatingSignedDifference(
                            static_cast<uint64_t>(
                                sample.RemoteResponseTransmitTime
                            ),
                            static_cast<uint64_t>(
                                sample.LocalResponseReceiveTime
                            )
                        );

                    const int64_t summed =
                        Internal::SaturatingSignedAdd(
                            remoteReceiveMinusLocalSend,
                            remoteSendMinusLocalReceive
                        );

                    offsetNanoseconds =
                        summed / 2;

                    return true;
                }


                void UpdateFilteredOffset(
                    int64_t measuredOffset
                ) {
                    const double weight =
                        Internal::NormalizeFilterWeight(
                            _config.OffsetFilterWeight
                        );

                    if (!_hasFilteredOffset) {
                        _filteredOffsetNanoseconds =
                            measuredOffset;

                        _hasFilteredOffset =
                            true;

                        return;
                    }

                    const double filtered =
                        (
                            1.0 -
                            weight
                        ) *
                            static_cast<double>(
                                _filteredOffsetNanoseconds
                            ) +
                        weight *
                            static_cast<double>(
                                measuredOffset
                            );

                    if (
                        filtered >=
                        static_cast<double>(
                            std::numeric_limits<int64_t>::max()
                        )
                    ) {
                        _filteredOffsetNanoseconds =
                            std::numeric_limits<int64_t>::max();
                    } else if (
                        filtered <=
                        static_cast<double>(
                            std::numeric_limits<int64_t>::min()
                        )
                    ) {
                        _filteredOffsetNanoseconds =
                            std::numeric_limits<int64_t>::min();
                    } else {
                        _filteredOffsetNanoseconds =
                            static_cast<int64_t>(
                                std::llround(
                                    filtered
                                )
                            );
                    }
                }


                void TryLearnDrift(
                    TTick localSampleTime,
                    int64_t measuredOffset
                ) {
                    const uint64_t phaseThreshold =
                        _config.
                            DriftLearningPhaseThresholdNanoseconds;

                    if (
                        Internal::AbsoluteSignedValue(
                            _pendingPhaseCorrectionNanoseconds
                        ) >
                            phaseThreshold ||
                        Internal::AbsoluteSignedValue(
                            measuredOffset
                        ) >
                            phaseThreshold
                    ) {
                        _hasPreviousDriftSample =
                            false;

                        return;
                    }

                    if (!_hasPreviousDriftSample) {
                        _previousDriftOffsetNanoseconds =
                            measuredOffset;

                        _previousDriftSampleLocalTime =
                            localSampleTime;

                        _hasPreviousDriftSample =
                            true;

                        return;
                    }

                    if (
                        localSampleTime <=
                        _previousDriftSampleLocalTime
                    ) {
                        _hasPreviousDriftSample =
                            false;

                        return;
                    }

                    const uint64_t interval =
                        static_cast<uint64_t>(
                            localSampleTime -
                            _previousDriftSampleLocalTime
                        );

                    if (
                        interval <
                        _config.
                            MinimumDriftLearningIntervalNanoseconds
                    ) {
                        return;
                    }

                    const int64_t signedOffsetChange =
                        Internal::SaturatingSignedSubtract(
                            measuredOffset,
                            _previousDriftOffsetNanoseconds
                        );

                    const double residualDriftPpm =
                        (
                            static_cast<double>(
                                signedOffsetChange
                            ) /
                            static_cast<double>(
                                interval
                            )
                        ) *
                        1000000.0;

                    /*
                     * The measured offset slope is the residual after the
                     * rate correction already being applied. Add the current
                     * correction back to estimate the underlying relative
                     * clock-rate error.
                     */
                    double measuredDriftPpm =
                        _estimatedDriftPpm +
                        residualDriftPpm;

                    measuredDriftPpm =
                        Internal::ClampDriftPpm(
                            measuredDriftPpm,
                            _config.
                                MaximumDriftCorrectionPpm
                        );

                    const double weight =
                        Internal::NormalizeFilterWeight(
                            _config.DriftFilterWeight
                        );

                    _estimatedDriftPpm =
                        (
                            1.0 -
                            weight
                        ) *
                            _estimatedDriftPpm +
                        weight *
                            measuredDriftPpm;

                    _estimatedDriftPpm =
                        Internal::ClampDriftPpm(
                            _estimatedDriftPpm,
                            _config.
                                MaximumDriftCorrectionPpm
                        );

                    _previousDriftOffsetNanoseconds =
                        measuredOffset;

                    _previousDriftSampleLocalTime =
                        localSampleTime;
                }


                int64_t ConsumeWholeCorrection(
                    double& fractional,
                    double correction
                ) {
                    fractional += correction;

                    if (
                        fractional > -1.0 &&
                        fractional < 1.0
                    ) {
                        return 0;
                    }

                    const double whole =
                        fractional >= 0.0
                            ? std::floor(
                                fractional
                              )
                            : std::ceil(
                                fractional
                              );

                    fractional -= whole;

                    if (
                        whole >=
                        static_cast<double>(
                            std::numeric_limits<int64_t>::max()
                        )
                    ) {
                        return
                            std::numeric_limits<int64_t>::max();
                    }

                    if (
                        whole <=
                        static_cast<double>(
                            std::numeric_limits<int64_t>::min()
                        )
                    ) {
                        return
                            std::numeric_limits<int64_t>::min();
                    }

                    return
                        static_cast<int64_t>(
                            whole
                        );
                }


            public:
                explicit ClockDiscipline(
                    const ClockSynchronizationConfig& config =
                        ClockSynchronizationConfig()
                )
                    : _config(config) {
                    Configure(
                        config
                    );
                }


                void Configure(
                    const ClockSynchronizationConfig& config
                ) {
                    _config = config;

                    _config.MaximumSlewRatePpm =
                        Internal::ClampSlewRatePpm(
                            _config.MaximumSlewRatePpm
                        );

                    _config.MaximumDriftCorrectionPpm =
                        std::max(
                            0.0,
                            std::min(
                                100000.0,
                                _config.MaximumDriftCorrectionPpm
                            )
                        );
                }


                const ClockSynchronizationConfig&
                GetConfig() const {
                    return _config;
                }


                void Reset(
                    bool preserveConfiguration =
                        true
                ) {
                    const ClockSynchronizationConfig
                        configuration =
                            _config;

                    *this =
                        ClockDiscipline<TTick>();

                    if (preserveConfiguration) {
                        Configure(
                            configuration
                        );
                    }
                }


                ClockSynchronizationResult<TTick>
                SubmitSample(
                    const ClockSynchronizationSample<TTick>& sample
                ) {
                    ClockSynchronizationResult<TTick>
                        result;

                    int64_t measuredOffset = 0;
                    uint64_t roundTripDelay = 0;

                    if (
                        !CalculateSample(
                            sample,
                            measuredOffset,
                            roundTripDelay
                        ) ||
                        (
                            _config.
                                MaximumRoundTripDelayNanoseconds >
                                0 &&
                            roundTripDelay >
                                _config.
                                    MaximumRoundTripDelayNanoseconds
                        )
                    ) {
                        ++_rejectedSampleCount;

                        result.RejectedSampleCount =
                            _rejectedSampleCount;

                        result.AcceptedSampleCount =
                            _acceptedSampleCount;

                        result.EstimatedDriftPpm =
                            _estimatedDriftPpm;

                        return result;
                    }

                    const bool phaseWasSettled =
                        _pendingPhaseCorrectionNanoseconds ==
                        0;

                    /*
                     * During acquisition, previous residual offsets become
                     * stale as the phase servo actively removes them. Use the
                     * latest measurement directly. Once settled, filtering is
                     * useful for radio/scheduler jitter.
                     */
                    if (
                        !_hasFilteredOffset ||
                        !phaseWasSettled
                    ) {
                        _filteredOffsetNanoseconds =
                            measuredOffset;

                        _hasFilteredOffset =
                            true;
                    } else {
                        UpdateFilteredOffset(
                            measuredOffset
                        );
                    }

                    if (phaseWasSettled) {
                        TryLearnDrift(
                            sample.LocalResponseReceiveTime,
                            measuredOffset
                        );
                    } else {
                        _hasPreviousDriftSample =
                            false;
                    }

                    _lastMeasuredOffsetNanoseconds =
                        measuredOffset;

                    _lastRoundTripDelayNanoseconds =
                        roundTripDelay;

                    _lastAcceptedSampleLocalTime =
                        sample.LocalResponseReceiveTime;

                    _hasAcceptedSample = true;

                    ++_acceptedSampleCount;

                    _pendingPhaseCorrectionNanoseconds =
                        _filteredOffsetNanoseconds;

                    result.Accepted = true;
                    result.MeasuredOffsetNanoseconds =
                        measuredOffset;
                    result.FilteredOffsetNanoseconds =
                        _filteredOffsetNanoseconds;
                    result.RoundTripDelayNanoseconds =
                        roundTripDelay;
                    result.EstimatedDriftPpm =
                        _estimatedDriftPpm;
                    result.AcceptedSampleCount =
                        _acceptedSampleCount;
                    result.RejectedSampleCount =
                        _rejectedSampleCount;

                    return result;
                }


                void ApplyStep(
                    int64_t correctionNanoseconds
                ) {
                    _appliedCorrectionNanoseconds =
                        Internal::SaturatingSignedAdd(
                            _appliedCorrectionNanoseconds,
                            correctionNanoseconds
                        );

                    _pendingPhaseCorrectionNanoseconds =
                        0;

                    _filteredOffsetNanoseconds =
                        0;

                    _lastMeasuredOffsetNanoseconds =
                        0;

                    _phaseFractionNanoseconds =
                        0.0;

                    if (_hasAcceptedSample) {
                        _lastAcceptedSampleLocalTime =
                            Internal::
                                ApplySignedTickCorrection(
                                    _lastAcceptedSampleLocalTime,
                                    correctionNanoseconds
                                );
                    }

                    if (_hasPreviousDriftSample) {
                        _previousDriftSampleLocalTime =
                            Internal::
                                ApplySignedTickCorrection(
                                    _previousDriftSampleLocalTime,
                                    correctionNanoseconds
                                );
                    }

                    _hasPreviousDriftSample =
                        false;
                }


                void Advance(
                    TTick rawTime
                ) {
                    if (!_advanceInitialized) {
                        _lastAdvanceRawTime =
                            rawTime;

                        _advanceInitialized =
                            true;

                        return;
                    }

                    if (
                        rawTime <=
                        _lastAdvanceRawTime
                    ) {
                        _lastAdvanceRawTime =
                            rawTime;

                        return;
                    }

                    const uint64_t elapsed =
                        static_cast<uint64_t>(
                            rawTime -
                            _lastAdvanceRawTime
                        );

                    _lastAdvanceRawTime =
                        rawTime;

                    /*
                     * Continuous rate correction.
                     */
                    const double frequencyCorrection =
                        (
                            static_cast<double>(
                                elapsed
                            ) *
                            _estimatedDriftPpm
                        ) /
                        1000000.0;

                    const int64_t wholeFrequencyCorrection =
                        ConsumeWholeCorrection(
                            _frequencyFractionNanoseconds,
                            frequencyCorrection
                        );

                    /*
                     * Phase servo.
                     */
                    int64_t wholePhaseCorrection = 0;

                    if (
                        _pendingPhaseCorrectionNanoseconds !=
                        0 &&
                        _config.MaximumSlewRatePpm >
                        0
                    ) {
                        const double maximumPhaseCorrection =
                            (
                                static_cast<double>(
                                    elapsed
                                ) *
                                static_cast<double>(
                                    Internal::ClampSlewRatePpm(
                                        _config.MaximumSlewRatePpm
                                    )
                                )
                            ) /
                            1000000.0;

                        double desiredPhaseCorrection =
                            _pendingPhaseCorrectionNanoseconds >
                                0
                                ? maximumPhaseCorrection
                                : -maximumPhaseCorrection;

                        if (
                            std::fabs(
                                desiredPhaseCorrection
                            ) >
                            static_cast<double>(
                                Internal::AbsoluteSignedValue(
                                    _pendingPhaseCorrectionNanoseconds
                                )
                            )
                        ) {
                            desiredPhaseCorrection =
                                static_cast<double>(
                                    _pendingPhaseCorrectionNanoseconds
                                );
                        }

                        wholePhaseCorrection =
                            ConsumeWholeCorrection(
                                _phaseFractionNanoseconds,
                                desiredPhaseCorrection
                            );

                        if (
                            wholePhaseCorrection != 0
                        ) {
                            if (
                                (
                                    _pendingPhaseCorrectionNanoseconds >
                                        0 &&
                                    wholePhaseCorrection >
                                        _pendingPhaseCorrectionNanoseconds
                                ) ||
                                (
                                    _pendingPhaseCorrectionNanoseconds <
                                        0 &&
                                    wholePhaseCorrection <
                                        _pendingPhaseCorrectionNanoseconds
                                )
                            ) {
                                wholePhaseCorrection =
                                    _pendingPhaseCorrectionNanoseconds;
                            }

                            _pendingPhaseCorrectionNanoseconds -=
                                wholePhaseCorrection;
                        }
                    }

                    const int64_t totalCorrection =
                        Internal::SaturatingSignedAdd(
                            wholeFrequencyCorrection,
                            wholePhaseCorrection
                        );

                    _appliedCorrectionNanoseconds =
                        Internal::SaturatingSignedAdd(
                            _appliedCorrectionNanoseconds,
                            totalCorrection
                        );
                }


                int64_t
                GetAppliedCorrectionNanoseconds() const {
                    return
                        _appliedCorrectionNanoseconds;
                }


                int64_t
                GetPendingPhaseCorrectionNanoseconds() const {
                    return
                        _pendingPhaseCorrectionNanoseconds;
                }


                double GetEstimatedDriftPpm() const {
                    return
                        _estimatedDriftPpm;
                }


                ClockSynchronizationStatus<TTick>
                GetStatus(
                    TTick currentLocalTime
                ) const {
                    ClockSynchronizationStatus<TTick>
                        status;

                    status.LastMeasuredOffsetNanoseconds =
                        _lastMeasuredOffsetNanoseconds;

                    status.FilteredOffsetNanoseconds =
                        _filteredOffsetNanoseconds;

                    status.PendingPhaseCorrectionNanoseconds =
                        _pendingPhaseCorrectionNanoseconds;

                    status.AppliedCorrectionNanoseconds =
                        _appliedCorrectionNanoseconds;

                    status.LastRoundTripDelayNanoseconds =
                        _lastRoundTripDelayNanoseconds;

                    status.EstimatedDriftPpm =
                        _estimatedDriftPpm;

                    status.AcceptedSampleCount =
                        _acceptedSampleCount;

                    status.RejectedSampleCount =
                        _rejectedSampleCount;

                    status.LastAcceptedSampleLocalTime =
                        _lastAcceptedSampleLocalTime;

                    status.HasAcceptedSample =
                        _hasAcceptedSample;

                    if (!_hasAcceptedSample) {
                        status.State =
                            ClockSynchronizationState::
                                Unsynchronized;

                        return status;
                    }

                    const bool stale =
                        _config.
                            MaximumSampleAgeNanoseconds >
                            0 &&
                        currentLocalTime >
                            _lastAcceptedSampleLocalTime &&
                        static_cast<uint64_t>(
                            currentLocalTime -
                            _lastAcceptedSampleLocalTime
                        ) >
                            _config.
                                MaximumSampleAgeNanoseconds;

                    if (stale) {
                        status.State =
                            ClockSynchronizationState::
                                Unsynchronized;

                        return status;
                    }

                    const bool enoughSamples =
                        _acceptedSampleCount >=
                        _config.
                            MinimumSamplesForSynchronizedState;

                    const bool phaseSettled =
                        Internal::AbsoluteSignedValue(
                            _pendingPhaseCorrectionNanoseconds
                        ) <=
                        _config.
                            SynchronizationToleranceNanoseconds;

                    status.State =
                        enoughSamples &&
                        phaseSettled
                            ? ClockSynchronizationState::
                                Synchronized
                            : ClockSynchronizationState::
                                Acquiring;

                    return status;
                }
        };

    }

}
