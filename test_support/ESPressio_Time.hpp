#pragma once

#include <cstdint>
#include <string>

namespace ESPressio {

    namespace Units {

/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
enum UnitOrderOfMagnitude : int8_t {
            Nano = -9,
            Micro = -6,
            Milli = -3,
            Base = 0
        };

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
class UnitContext : uint8_t {
            Time
        };

/**
 * ESPressio Memory Audit
 * Members:
 * - orderOfMagnitude (UnitOrderOfMagnitude): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
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
