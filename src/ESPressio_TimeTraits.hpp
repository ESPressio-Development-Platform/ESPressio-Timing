#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include "ESPressio_ClockTypes.hpp"

namespace ESPressio {

    namespace Timing {

        namespace Internal {

            template<typename...>
            struct DependentFalse : std::false_type {
            };

            inline Units::UnitOrderOfMagnitude
            GetMagnitudeForResolution(
                ClockTick resolution
            ) {
                if (
                    resolution >= NanosecondsPerSecond &&
                    resolution % NanosecondsPerSecond == 0
                ) {
                    return Units::Base;
                }

                if (
                    resolution >= NanosecondsPerMillisecond &&
                    resolution % NanosecondsPerMillisecond == 0
                ) {
                    return Units::Milli;
                }

                if (
                    resolution >= NanosecondsPerMicrosecond &&
                    resolution % NanosecondsPerMicrosecond == 0
                ) {
                    return Units::Micro;
                }

                return Units::Nano;
            }

            inline ClockTick GetMagnitudeScale(
                Units::UnitOrderOfMagnitude magnitude
            ) {
                switch (magnitude) {
                    case Units::Base:
                        return NanosecondsPerSecond;

                    case Units::Milli:
                        return NanosecondsPerMillisecond;

                    case Units::Micro:
                        return NanosecondsPerMicrosecond;

                    default:
                        return 1;
                }
            }

        }


        /*
         * Customization point connecting a public clock-time representation
         * with Timing's nanosecond tick domain.
         *
         * The default specialization accepts Unit-like types exposing:
         *
         *   value
         *   orderOfMagnitude
         *   TTime(value, magnitude)
         *
         * This intentionally covers both Units::Time and its opt-in
         * Serializable wrapper without Timing depending upon Serializable.
         *
         * Applications may explicitly specialize TimeTraits<T> for unrelated
         * custom time representations.
         */
        template<typename TTime, typename TEnable = void>
        struct TimeTraits {
            static_assert(
                Internal::DependentFalse<TTime>::value,
                "ESPressio Timing: TTime is not a supported time representation. "
                "Use an ESPressio Units Time-compatible type or specialize "
                "ESPressio::Timing::TimeTraits<TTime>."
            );
        };


        template<typename TTime>
        struct TimeTraits<
            TTime,
            std::void_t<
                decltype(
                    std::declval<TTime>().value
                ),
                decltype(
                    std::declval<TTime>().orderOfMagnitude
                )
            >
        > {
            using TimeType = TTime;

            using ValueType =
                std::remove_cv_t<
                    std::remove_reference_t<
                        decltype(
                            std::declval<TTime>().value
                        )
                    >
                >;


            template<typename TTick = ClockTick>
            static TTime FromNanoseconds(
                TTick nanoseconds,
                TTick resolution
            ) {
                const auto magnitude =
                    Internal::GetMagnitudeForResolution(
                        static_cast<ClockTick>(
                            resolution
                        )
                    );

                const ClockTick scale =
                    Internal::GetMagnitudeScale(
                        magnitude
                    );

                return TTime(
                    static_cast<ValueType>(
                        static_cast<ClockTick>(
                            nanoseconds
                        ) /
                        scale
                    ),
                    magnitude
                );
            }


            template<typename TTick = ClockTick>
            static TTick ToNanoseconds(
                const TTime& time
            ) {
                const int exponentDifference =
                    static_cast<int>(
                        time.orderOfMagnitude
                    ) -
                    static_cast<int>(
                        Units::Nano
                    );

                using UnsignedTick =
                    std::make_unsigned_t<TTick>;

                UnsignedTick value =
                    static_cast<UnsignedTick>(
                        time.value
                    );

                if (exponentDifference < 0) {
                    for (
                        int exponent =
                            exponentDifference;
                        exponent < 0;
                        ++exponent
                    ) {
                        value /= 10;
                    }

                    return static_cast<TTick>(
                        value
                    );
                }

                const UnsignedTick maximum =
                    static_cast<UnsignedTick>(
                        std::numeric_limits<
                            TTick
                        >::max()
                    );

                for (
                    int exponent = 0;
                    exponent < exponentDifference;
                    ++exponent
                ) {
                    if (value > maximum / 10) {
                        return std::numeric_limits<
                            TTick
                        >::max();
                    }

                    value *= 10;
                }

                return static_cast<TTick>(
                    value
                );
            }
        };

    }

}
