#pragma once
#include <functional>
#include "ESPressio_IClock.hpp"
#include "ESPressio_ClockSynchronization.hpp"
namespace ESPressio::Timing {
/// <summary>Continuous System clock with an explicit bootstrap-only rebase and separately serviced callbacks.</summary>
/// <remarks>It cannot implement unconditional IClockSettable after the one-way continuity seal.</remarks>
template<class TTime=DefaultClockTime> class ISystemClock : public virtual IClock<TTime> {
public:
    using TimeType=TTime;
    using ClockCallback=std::function<void()>;
    virtual ClockConfigurationStatus TrySetTime(const TTime& time)=0;
    virtual void SealContinuity()=0;
    virtual bool IsContinuitySealed() const=0;
    virtual void SetCallback(const TTime& time,ClockCallback callback)=0;
    /// <summary>Explicitly services due callbacks and diagnostic transitions; ordinary reads never do this work.</summary>
    virtual void Update()=0;
    virtual void ClearCallbacks()=0;
};
}
