#pragma once

#include "ESPressio_Time.hpp"

namespace ESPressio {

    namespace Units {

        template<typename TValue>
        struct SerializableNanoSeconds :
            public NanoSeconds<TValue> {

            using NanoSeconds<TValue>::
                NanoSeconds;

            bool SerializableMarker = true;
        };

    }

}
