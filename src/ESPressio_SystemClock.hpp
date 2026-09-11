#pragma once
#include <array>
#include <exception>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <ESPressio_SystemPlatformClock.hpp>
#include "ESPressio_ClockDiscipline.hpp"
#include "ESPressio_ISystemClock.hpp"
#include "ESPressio_IClockSynchronizationTarget.hpp"
#include "ESPressio_ISystemClockObserver.hpp"
#include "ESPressio_TimingObservable.hpp"
#include "ESPressio_TimingObserverUtilities.hpp"
#include "ESPressio_TimeSource.hpp"
#include "ESPressio_TimeTraits.hpp"
#include "ESPressio_LockPolicy.hpp"
#include "ESPressio_ThreadSafeLockPolicy.hpp"
#ifndef ESPRESSIO_TIMING_MAX_CALLBACKS
#define ESPRESSIO_TIMING_MAX_CALLBACKS 8
#endif
namespace ESPressio::Timing {
/// <summary>Canonical raw source adapter; synchronization never changes System::Clock::Monotonic.</summary>
class SystemMonotonicTimeSource final : public ITimeSource {
public:
    std::uint64_t GetTicks() const override { return System::Clock::Monotonic().NowNanoseconds(); }
    std::uint64_t GetTicksPerSecond() const override { return 1000000000ull; }
    static SystemMonotonicTimeSource& Instance() noexcept { static SystemMonotonicTimeSource source; return source; }
};
/// <summary>One shared nanosecond timeline/model across all typed facades with the same lock/tick policy.</summary>
/// <remarks>Time/status/capture reads copy/evaluate fixed state under a short lock and never advance discipline or notify.
/// Explicit operations own estimator work and optional Observable diagnostics. Callback closures are an explicit general
/// scheduler convenience, outside the bounded synchronization model and Primitive hot path.</remarks>
template<class TLockPolicy=ThreadSafeLockPolicy,class TTick=ClockTick> class SystemClockCore final {
    static_assert(std::is_same_v<TTick,std::uint64_t>,"The distributed System timeline uses canonical uint64 nanoseconds");
    using Callback=std::function<void()>;
    struct Scheduled { TTick Time=0; Callback Function; };
    ITimeSource* _source;
    mutable typename TLockPolicy::Mutex _clockMutex,_callbacksMutex;
    ClockDiscipline<8> _discipline;
    std::shared_ptr<TimingObservable> _observable=CreateTimingObservable();
    TimeReliability _notified=TimeReliability::Unqualified;
    std::array<Scheduled,ESPRESSIO_TIMING_MAX_CALLBACKS> _callbacks{};
    explicit SystemClockCore(ITimeSource* source):_source(source ? source : &SystemMonotonicTimeSource::Instance()) {}
    TTick Raw() const { return Internal::TicksToNanoseconds(_source->GetTicks(),_source->GetTicksPerSecond()); }
    void NotifyState(TimeReliability before,const ClockSynchronizationStatus& status) {
        if (before==status.Reliability) return;
        _observable->template Notify<ISystemClockObserver<TTick>>([&](auto* observer) {
            observer->OnSystemClockSynchronizationStateChanged(before,status.Reliability,status);
        });
    }
public:
    SystemClockCore(const SystemClockCore&)=delete;
    SystemClockCore& operator=(const SystemClockCore&)=delete;
    static SystemClockCore& GetInstance(ITimeSource* source=nullptr) { static SystemClockCore instance(source); return instance; }
    /// <summary>Evaluates one published model at its raw source coordinate; no callback, allocation or state update.</summary>
    TTick GetTimeNanoseconds() const {
        typename TLockPolicy::Guard lock(_clockMutex);
        return _discipline.Model().Evaluate(Raw());
    }
    TTick GetResolutionNanoseconds() const {
        if (_source==&SystemMonotonicTimeSource::Instance()) return System::Clock::Monotonic().ResolutionNanoseconds();
        return Internal::GetSourceResolution(_source->GetTicksPerSecond());
    }
    /// <summary>Captures semantic origin time and reliability together, before any family-owned bounded wait.</summary>
    QualifiedTime CaptureQualifiedTime() const {
        typename TLockPolicy::Guard lock(_clockMutex); const auto now=Raw();
        return {_discipline.Model().Evaluate(now),_discipline.GetStatus(now).Reliability};
    }
    ClockTimestampCapture<> CaptureSynchronizationTimestamp(ClockCaptureQuality quality,ClockUncertainty uncertainty) const {
        typename TLockPolicy::Guard lock(_clockMutex); const auto now=Raw();
        return {_discipline.Model().Evaluate(now),now,uncertainty,quality};
    }
    /// <summary>Copies the current mapping for a provider that retains capture-consistent model evidence.</summary>
    ClockModelSnapshot GetClockModelSnapshot() const { typename TLockPolicy::Guard lock(_clockMutex); return _discipline.Model(); }
    ClockConfigurationStatus TrySetTimeNanoseconds(TTick time) {
        TTick previous=0; ClockSynchronizationStatus before,after; ClockConfigurationStatus result;
        {
            typename TLockPolicy::Guard lock(_clockMutex); const auto now=Raw();
            previous=_discipline.Model().Evaluate(now); before=_discipline.GetStatus(now);
            result=_discipline.TryRebase(time,now);
            if (result!=ClockConfigurationStatus::Success) return result;
            after=_discipline.GetStatus(now); _notified=after.Reliability;
        }
        _observable->template Notify<ISystemClockObserver<TTick>>([&](auto* observer) {
            observer->OnSystemClockTimeSet(previous,time,Internal::SignedDifference(time,previous));
            observer->OnSystemClockSynchronizationReset(before,after);
        });
        NotifyState(before.Reliability,after); return result;
    }
    void SealContinuity() { typename TLockPolicy::Guard lock(_clockMutex); _discipline.SealContinuity(); }
    bool IsContinuitySealed() const { typename TLockPolicy::Guard lock(_clockMutex); return _discipline.IsContinuitySealed(); }
    ClockSynchronizationResult SubmitSynchronizationObservation(const ClockSynchronizationObservation<>& observation) {
        ClockSynchronizationResult result; ClockSynchronizationStatus status; TimeReliability previous; TTick time;
        {
            typename TLockPolicy::Guard lock(_clockMutex); const auto now=Raw();
            time=_discipline.Model().Evaluate(now); previous=_notified;
            result=_discipline.Submit(observation,now,GetResolutionNanoseconds());
            status=_discipline.GetStatus(now); _notified=status.Reliability;
        }
        _observable->template Notify<ISystemClockObserver<TTick>>([&](auto* observer) {
            if (result.Accepted) {
                observer->OnSystemClockSynchronizationSampleAccepted(time,time,0,result,status);
                if (status.Reliability==TimeReliability::Synchronized && previous!=TimeReliability::Synchronized)
                    observer->OnSystemClockSynchronized(time,time,0,result,status);
            } else observer->OnSystemClockSynchronizationSampleRejected(result,status);
        });
        NotifyState(previous,status); return result;
    }
    ClockSynchronizationStatus GetSynchronizationStatus() const { typename TLockPolicy::Guard lock(_clockMutex); return _discipline.GetStatus(Raw()); }
    ClockSynchronizationProfile GetSynchronizationProfile() const { typename TLockPolicy::Guard lock(_clockMutex); return _discipline.GetProfile(); }
    ClockConfigurationStatus ConfigureSynchronization(const ClockSynchronizationProfile& profile) {
        ClockSynchronizationProfile previousProfile; ClockSynchronizationStatus status; TimeReliability previous;
        ClockConfigurationStatus result;
        {
            typename TLockPolicy::Guard lock(_clockMutex); previousProfile=_discipline.GetProfile(); previous=_notified;
            const auto now=Raw(); result=_discipline.Configure(profile,now);
            if (result!=ClockConfigurationStatus::Success) return result;
            status=_discipline.GetStatus(now); _notified=status.Reliability;
        }
        _observable->template Notify<ISystemClockObserver<TTick>>([&](auto* observer) {
            observer->OnSystemClockSynchronizationConfigurationChanged(previousProfile,profile);
        });
        NotifyState(previous,status); return result;
    }
    ClockConfigurationStatus SelectSynchronizationReference(std::uint64_t reference) {
        ClockSynchronizationStatus status; TimeReliability previous; ClockConfigurationStatus result;
        {
            typename TLockPolicy::Guard lock(_clockMutex); const auto now=Raw(); previous=_notified;
            result=_discipline.SelectReference(reference,now); status=_discipline.GetStatus(now); _notified=status.Reliability;
        }
        NotifyState(previous,status); return result;
    }
    void SetSynchronizationActivity(bool acquiring,bool referenceAvailable) {
        ClockSynchronizationStatus status; TimeReliability previous;
        { typename TLockPolicy::Guard lock(_clockMutex); const auto now=Raw(); previous=_notified;
          _discipline.SetActivity(acquiring,referenceAvailable,now); status=_discipline.GetStatus(now); _notified=status.Reliability; }
        NotifyState(previous,status);
    }
    void ResetSynchronization() {
        ClockSynchronizationStatus before,after;
        { typename TLockPolicy::Guard lock(_clockMutex); const auto now=Raw(); before=_discipline.GetStatus(now);
          _discipline.Reset(now); after=_discipline.GetStatus(now); _notified=after.Reliability; }
        _observable->template Notify<ISystemClockObserver<TTick>>([&](auto* observer) { observer->OnSystemClockSynchronizationReset(before,after); });
        NotifyState(before.Reliability,after);
    }
    void RecordSynchronizationDeadlineMiss() { typename TLockPolicy::Guard lock(_clockMutex); _discipline.RecordSchedulerDeadlineMiss(); }
    bool TrySetCallbackNanoseconds(TTick time,Callback callback) {
        bool accepted=false;
        if (callback) {
            typename TLockPolicy::Guard lock(_callbacksMutex);
            for (auto& slot:_callbacks) if (!slot.Function) { slot.Time=time; slot.Function=std::move(callback); accepted=true; break; }
        }
        _observable->template Notify<ISystemClockObserver<TTick>>([&](auto* observer) {
            if (accepted) observer->OnSystemClockCallbackScheduled(time); else observer->OnSystemClockCallbackScheduleFailed(time);
        });
        return accepted;
    }
    /// <summary>Explicit service point for reliability diagnostics and bounded due-callback extraction.</summary>
    void Update() {
        ClockSynchronizationStatus status; TimeReliability previous; TTick now;
        { typename TLockPolicy::Guard lock(_clockMutex); const auto raw=Raw(); now=_discipline.Model().Evaluate(raw);
          status=_discipline.GetStatus(raw); previous=_notified; _notified=status.Reliability; }
        NotifyState(previous,status);
        std::array<Scheduled,ESPRESSIO_TIMING_MAX_CALLBACKS> due{};
        { typename TLockPolicy::Guard lock(_callbacksMutex);
          for (std::size_t i=0;i<_callbacks.size();++i) if (_callbacks[i].Function && now>=_callbacks[i].Time) {
              due[i]=std::move(_callbacks[i]); _callbacks[i]={};
          } }
        for (auto& callback:due) if (callback.Function) {
            try { callback.Function(); }
            catch (...) {
                const auto actual=GetTimeNanoseconds(); const auto cause=std::current_exception();
                _observable->template Notify<ISystemClockObserver<TTick>>([&](auto* observer) {
                    observer->OnSystemClockCallbackExecutionFailed(callback.Time,actual,Internal::SignedDifference(actual,callback.Time),cause);
                });
                std::rethrow_exception(cause);
            }
            const auto actual=GetTimeNanoseconds();
            _observable->template Notify<ISystemClockObserver<TTick>>([&](auto* observer) {
                observer->OnSystemClockCallbackExecuted(callback.Time,actual,Internal::SignedDifference(actual,callback.Time));
            });
        }
    }
    void ClearCallbacks() {
        std::size_t count=0;
        { typename TLockPolicy::Guard lock(_callbacksMutex); for (auto& slot:_callbacks) { if (slot.Function) ++count; slot={}; } }
        if (count) _observable->template Notify<ISystemClockObserver<TTick>>([&](auto* observer) { observer->OnSystemClockCallbacksCleared(count); });
    }
    Observable::ObserverHandlePtr RegisterObserver(ISystemClockObserver<TTick>* observer) { return _observable->RegisterObserver(observer); }
    void UnregisterObserver(ISystemClockObserver<TTick>* observer) { _observable->UnregisterObserver(observer); }
    ITimeSource* GetTimeSource() const noexcept { return _source; }
};
/// <summary>Unit-aware view of the shared sealed System timeline; representation never owns a separate estimator.</summary>
template<class TTime=DefaultClockTime,class TLockPolicy=ThreadSafeLockPolicy,class TTick=ClockTick>
class SystemClock : public ISystemClock<TTime>,public IClockSynchronizationTarget {
    using Core=SystemClockCore<TLockPolicy,TTick>;
    Core& _core;
    explicit SystemClock(ITimeSource* source):_core(Core::GetInstance(source)) {}
public:
    using TimeType=TTime; using TickType=TTick;
    using ClockCallback=typename ISystemClock<TTime>::ClockCallback;
    SystemClock(const SystemClock&)=delete; SystemClock& operator=(const SystemClock&)=delete;
    static SystemClock& GetInstance(ITimeSource* source=nullptr) { static SystemClock instance(source); return instance; }
    TTime GetTime() const override { return TimeTraits<TTime>::template FromNanoseconds<TTick>(_core.GetTimeNanoseconds(),_core.GetResolutionNanoseconds()); }
    TTime GetResolution() const override { const auto resolution=_core.GetResolutionNanoseconds(); return TimeTraits<TTime>::template FromNanoseconds<TTick>(resolution,resolution); }
    ClockConfigurationStatus TrySetTime(const TTime& time) override { return _core.TrySetTimeNanoseconds(TimeTraits<TTime>::template ToNanoseconds<TTick>(time)); }
    void SealContinuity() override { _core.SealContinuity(); }
    bool IsContinuitySealed() const override { return _core.IsContinuitySealed(); }
    QualifiedTime CaptureQualifiedTime() const { return _core.CaptureQualifiedTime(); }
    ClockModelSnapshot GetClockModelSnapshot() const { return _core.GetClockModelSnapshot(); }
    ClockTimestampCapture<> CaptureSynchronizationTimestamp(ClockCaptureQuality quality=ClockCaptureQuality::SoftwareUnbounded,ClockUncertainty uncertainty={}) const override {
        return _core.CaptureSynchronizationTimestamp(quality,uncertainty);
    }
    ClockSynchronizationResult SubmitSynchronizationObservation(const ClockSynchronizationObservation<>& observation) override { return _core.SubmitSynchronizationObservation(observation); }
    ClockSynchronizationStatus GetSynchronizationStatus() const override { return _core.GetSynchronizationStatus(); }
    ClockConfigurationStatus ConfigureSynchronization(const ClockSynchronizationProfile& profile) override { return _core.ConfigureSynchronization(profile); }
    ClockSynchronizationProfile GetSynchronizationProfile() const override { return _core.GetSynchronizationProfile(); }
    ClockConfigurationStatus SelectSynchronizationReference(std::uint64_t reference) override { return _core.SelectSynchronizationReference(reference); }
    void SetSynchronizationActivity(bool acquiring,bool available) override { _core.SetSynchronizationActivity(acquiring,available); }
    void ResetSynchronization() override { _core.ResetSynchronization(); }
    void RecordSynchronizationDeadlineMiss() override { _core.RecordSynchronizationDeadlineMiss(); }
    bool TrySetCallback(const TTime& time,ClockCallback callback) { return _core.TrySetCallbackNanoseconds(TimeTraits<TTime>::template ToNanoseconds<TTick>(time),std::move(callback)); }
    void SetCallback(const TTime& time,ClockCallback callback) override { (void)TrySetCallback(time,std::move(callback)); }
    void Update() override { _core.Update(); }
    void ClearCallbacks() override { _core.ClearCallbacks(); }
    Observable::ObserverHandlePtr RegisterObserver(ISystemClockObserver<TTick>* observer) { return _core.RegisterObserver(observer); }
    void UnregisterObserver(ISystemClockObserver<TTick>* observer) { _core.UnregisterObserver(observer); }
    ITimeSource* GetTimeSource() const noexcept { return _core.GetTimeSource(); }
};
template<class TTime=DefaultClockTime,class TTick=ClockTick>
using SingleThreadedSystemClock=SystemClock<TTime,NoLockPolicy,TTick>;
}
