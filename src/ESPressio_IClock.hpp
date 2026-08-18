#pragma once

#include "ESPressio_ClockTypes.hpp"

namespace ESPressio {

    namespace Timing {

        /*
         * Common clock contract parameterized by the public time
         * representation.
         *
         * A clock's TimeType is part of its interface rather than a global
         * library-wide typedef.
         */
        template<typename TTime = DefaultClockTime>
        class IClock {
            public:
                using TimeType = TTime;

                virtual ~IClock() = default;

                virtual TTime GetTime() const = 0;
                virtual TTime GetResolution() const = 0;
        };


        template<typename TTime = DefaultClockTime>
        class IClockSettable :
            public virtual IClock<TTime> {

            public:
                using TimeType = TTime;

                virtual void SetTime(
                    const TTime& time
                ) = 0;
        };

    }

}
