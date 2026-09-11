#pragma once
#include "ESPressio_ClockSynchronization.hpp"
namespace ESPressio::Timing {
/// <summary>Transport-neutral capture, reference and observation contract. Timing alone owns validation and qualification.</summary>
class IClockSynchronizationTarget {
public:
    virtual ~IClockSynchronizationTarget()=default;
    /// <summary>Captures a coherent current System/raw-monotonic pair; unknown latency is acquisition-only.</summary>
    virtual ClockTimestampCapture<> CaptureSynchronizationTimestamp(
        ClockCaptureQuality quality=ClockCaptureQuality::SoftwareUnbounded,ClockUncertainty uncertainty={}) const=0;
    virtual ClockSynchronizationResult SubmitSynchronizationObservation(const ClockSynchronizationObservation<>& observation)=0;
    virtual ClockSynchronizationStatus GetSynchronizationStatus() const=0;
    virtual ClockConfigurationStatus ConfigureSynchronization(const ClockSynchronizationProfile& profile)=0;
    virtual ClockSynchronizationProfile GetSynchronizationProfile() const=0;
    virtual ClockConfigurationStatus SelectSynchronizationReference(std::uint64_t reference)=0;
    virtual void SetSynchronizationActivity(bool acquiring,bool referenceAvailable)=0;
    virtual void ResetSynchronization()=0;
    virtual void RecordSynchronizationDeadlineMiss()=0;
};
}
