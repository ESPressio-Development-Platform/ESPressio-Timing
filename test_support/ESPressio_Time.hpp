#pragma once

#include <cstdint>
#include <string>

namespace ESPressio {

    namespace Units {

/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
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
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
enum
class UnitContext : uint8_t {
            Time
        };

/**
 * ESPressio Memory Audit
 * Members:
 * - value (TValue): sizeof(TValue) [0 bytes dynamic allocation]
 * - orderOfMagnitude (UnitOrderOfMagnitude): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: 1 bytes known/aligned storage + sizeof(TValue) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
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
