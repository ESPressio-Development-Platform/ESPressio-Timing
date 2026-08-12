#pragma once

#include <ESPressio_ISystemClock.hpp>

namespace ESPressio {

    namespace Timing {

        class SystemClock : public ISystemClock {
            protected:
                SystemClock() {

                }

            public:
                static ISystemClock* GetInstance() {
                    static ISystemClock* instance = new SystemClock();
                    return instance;
                }

            // Getters

                uint64_t GetTime() {
                    return 0; //TODO: Implement this
                }

            // Setters

                void SetTime(uint64_t time) {
                    //TODO: Implement this
                }

            // Methods

                void SetCallback(
                    uint64_t time,
                    std::function<void()> callback
                ) {
                    //TODO: Implement this
                }
        };

    }

}