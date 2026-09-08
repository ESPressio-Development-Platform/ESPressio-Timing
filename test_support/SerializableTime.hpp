#pragma once

#include "ESPressio_Time.hpp"

namespace ESPressio {

    namespace Units {

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes known/aligned storage + sizeof(TValue) [0 bytes dynamic allocation]
 * Members:
 * - SerializableMarker (bool): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: 1 bytes known/aligned storage + 1 bytes known/aligned storage + sizeof(TValue) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
template<typename TValue>
        struct SerializableNanoSeconds :
            public NanoSeconds<TValue> {

            using NanoSeconds<TValue>::
                NanoSeconds;

            bool SerializableMarker = true;
        };

    }

}
