#pragma once

#include <cstdint>
#include <string>

namespace ESPressio {

    namespace Units {

        enum UnitOrderOfMagnitude : int8_t {
            Nano = -9,
            Micro = -6,
            Milli = -3,
            Base = 0
        };

        enum class UnitContext : uint8_t {
            Time
        };

        template<
            typename TValue,
            UnitOrderOfMagnitude TBase =
                Nano
        >
        struct Time {
            TValue value {};
            UnitOrderOfMagnitude
                orderOfMagnitude =
                    TBase;

            static constexpr auto context =
                UnitContext::Time;

            Time() = default;

            explicit Time(
                TValue value,
                UnitOrderOfMagnitude magnitude =
                    TBase
            )
                : value(value),
                  orderOfMagnitude(
                      magnitude
                  ) {
            }
        };

        template<typename TValue>
        using NanoSeconds =
            Time<TValue, Nano>;

    }

}
