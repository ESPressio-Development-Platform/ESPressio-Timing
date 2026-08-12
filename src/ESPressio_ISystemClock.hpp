#pragma once

#include <ESPressio_IClock.hpp>

namespace ESPressio {

    namespace Timing {

        class ISystemClock {
            public:

            // Getters

                /*
                    Get the time of the global clock.
                */
                virtual uint64_t GetTime() = 0;

            // Setters

                /*
                    Set the time of the global clock.
                */
                virtual void SetTime(uint64_t time) = 0;

            // Methods

                /*
                    Set a callback to be executed at the given time
                */
                virtual void SetCallback(
                    uint64_t time, /* The time at which the callback should be invoked */
                    std::function<void()> callback /* The callback to be invoked */
                ) = 0;
        };

    }

}