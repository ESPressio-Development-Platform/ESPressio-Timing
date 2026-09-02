#pragma once

#include "ESPressio_ClockTypes.hpp"

namespace ESPressio {

    namespace Timing {

        /// <summary>Common clock contract parameterized by the public time representation.</summary>
        /// <typeparam name="TTime">Unit-aware value type returned by the clock.</typeparam>
        /// <remarks>A clock's <c>TimeType</c> is part of its interface rather than a library-wide global typedef.</remarks>
        template<typename TTime = DefaultClockTime>
        class IClock {
            public:
                /// <summary>Public time representation exposed by the clock.</summary>
                using TimeType = TTime;

                virtual ~IClock() = default;

                /// <summary>Returns the clock's current time value.</summary>
                virtual TTime GetTime() const = 0;
                /// <summary>Returns the smallest time increment represented by the clock.</summary>
                virtual TTime GetResolution() const = 0;
        };


        /// <summary>Clock contract for implementations whose current time can be explicitly assigned.</summary>
        template<typename TTime = DefaultClockTime>
        class IClockSettable :
            public virtual IClock<TTime> {

            public:
                using TimeType = TTime;

                /// <summary>Sets the clock to the supplied time value.</summary>
                virtual void SetTime(
                    const TTime& time
                ) = 0;
        };

    }

}
