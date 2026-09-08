#pragma once

#include "ESPressio_Time.hpp"

namespace ESPressio {

    namespace Units {

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(NanoSeconds<TValue>) [0 bytes dynamic allocation]
 * Members:
 * - SerializableMarker (bool): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: sizeof(NanoSeconds<TValue>) + 1 bytes known members [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
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
