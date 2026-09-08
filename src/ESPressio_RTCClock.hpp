#pragma once

#include <memory>

#include "ESPressio_Clock.hpp"
#include "ESPressio_IRTCClock.hpp"
#include "ESPressio_IRTCClockObserver.hpp"
#include "ESPressio_LockPolicy.hpp"
#include "ESPressio_ThreadSafeLockPolicy.hpp"
#include "ESPressio_TimingObservable.hpp"
#include "ESPressio_TimingObserverUtilities.hpp"

namespace ESPressio {
namespace Timing {

/// <summary>Observable RTC-backed clock base that extrapolates between RTC synchronization points using a monotonic time source.</summary>
/// <typeparam name="TLockPolicy">Synchronization policy protecting clock state and RTC I/O.</typeparam>
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 12 bytes [0 bytes dynamic allocation]
 * Members:
 * - _rtcTime (TTick): sizeof(TTick) [0 bytes dynamic allocation]
 * - _sourceTime (TTick): sizeof(TTick) [0 bytes dynamic allocation]
 * - _rtcResolution (TTick): sizeof(TTick) [0 bytes dynamic allocation]
 * - _isSynchronized (bool): 1 bytes [0 bytes dynamic allocation]
 * - _stateMutex (TLockPolicy::Mutex): 1 bytes [0 bytes dynamic allocation]
 * - _rtcIOMutex (TLockPolicy::Mutex): 1 bytes [0 bytes dynamic allocation]
 * - _observable (std::shared_ptr<TimingObservable>): 8 bytes [shared control block (~12+ bytes; allocate_shared may co-locate object) + object 20 bytes; pointee: ThreadSafeObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; pointee: ThreadSafeObservable: mutex_: native synchronization state may allocate platform resources lazily]
 * Total Memory: 24 bytes known/aligned storage + sizeof(TTick) + sizeof(TTick) + sizeof(TTick) [_observable: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 20 bytes; _observable: pointee: ThreadSafeObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; _observable: pointee: ThreadSafeObservable: mutex_: native synchronization state may allocate platform resources lazily]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
template<
    typename TTime = DefaultClockTime,
    typename TLockPolicy = ThreadSafeLockPolicy,
    typename TTick = ClockTick
>
class RTCClockBase :
    public ClockBase<TTime, TTick>,
    public IRTCClock<TTime> {
private:
    using Base = ClockBase<TTime, TTick>;

    TTick _rtcTime = 0;
    TTick _sourceTime = 0;
    TTick _rtcResolution;
    bool _isSynchronized = false;

    mutable typename TLockPolicy::Mutex _stateMutex;
    mutable typename TLockPolicy::Mutex _rtcIOMutex;

    std::shared_ptr<TimingObservable> _observable =
        CreateTimingObservable();

    TTick GetTimeNanosecondsLocked(
        TTick sourceTime
    ) const {
        if (!_isSynchronized) {
            return 0;
        }

        const TTick elapsed =
            sourceTime >= _sourceTime
                ? sourceTime - _sourceTime
                : 0;

        return this->AddSaturated(
            _rtcTime,
            elapsed
        );
    }

    void ApplySynchronizedTime(
        const TTime& time,
        TTick sourceTime,
        bool notifySynchronization
    ) {
        const TTick synchronizedTime =
            this->GetNanoseconds(time);

        TTick previousTime = 0;

        {
            typename TLockPolicy::Guard lock(_stateMutex);

            previousTime =
                GetTimeNanosecondsLocked(sourceTime);

            _sourceTime = sourceTime;
            _rtcTime = synchronizedTime;
            _isSynchronized = true;
        }

        if (notifySynchronization) {
            _observable->Notify<
                IRTCClockObserver<TTime, TTick>
            >(
                [&](IRTCClockObserver<TTime, TTick>* observer) {
                    observer->OnRTCSynchronizationSucceeded(
                        previousTime,
                        synchronizedTime,
                        Internal::SignedDifference(
                            synchronizedTime,
                            previousTime
                        )
                    );
                }
            );
        }
    }

protected:
    /// <summary>Creates an RTC-backed clock with the supplied RTC resolution and monotonic extrapolation source.</summary>
    explicit RTCClockBase(
        const TTime& rtcResolution,
        ITimeSource* timeSource =
            HighResolutionTimeSourceT<TLockPolicy>::GetInstance()
    ) : Base(timeSource),
        _rtcResolution(
            this->GetNanoseconds(rtcResolution)
        ) {
    }

    /// <summary>Reads the current time from the concrete RTC implementation.</summary>
    virtual bool ReadRTC(TTime& time) = 0;
    /// <summary>Writes a time value to the concrete RTC implementation.</summary>
    virtual bool WriteRTC(const TTime& time) = 0;

    /// <summary>Applies an already acquired RTC time as a synchronized clock point.</summary>
    void SetSynchronizedTime(
        const TTime& time
    ) {
        const TTick sourceTime = this->GetSourceTime();
        ApplySynchronizedTime(
            time,
            sourceTime,
            true
        );
    }

public:
    using TimeType = TTime;
    using TickType = TTick;

    /// <inheritdoc/>
    bool Synchronize() override {
        TTime time;
        bool readSucceeded = false;

        {
            typename TLockPolicy::Guard ioLock(_rtcIOMutex);
            readSucceeded = ReadRTC(time);
        }

        if (!readSucceeded) {
            _observable->Notify<
                IRTCClockObserver<TTime, TTick>
            >(
                [](IRTCClockObserver<TTime, TTick>* observer) {
                    observer->OnRTCSynchronizationFailed();
                }
            );

            return false;
        }

        const TTick sourceTime = this->GetSourceTime();

        ApplySynchronizedTime(
            time,
            sourceTime,
            true
        );

        return true;
    }

    /// <inheritdoc/>
    void OnRTCInterrupt() override {
        _observable->Notify<
            IRTCClockObserver<TTime, TTick>
        >(
            [](IRTCClockObserver<TTime, TTick>* observer) {
                observer->OnRTCInterruptReceived();
            }
        );

        Synchronize();
    }

    /// <inheritdoc/>
    void OnRTCInterrupt(
        const TTime& time
    ) override {
        const TTick suppliedTime =
            this->GetNanoseconds(time);

        _observable->Notify<
            IRTCClockObserver<TTime, TTick>
        >(
            [&](IRTCClockObserver<TTime, TTick>* observer) {
                observer->OnRTCInterruptTimeReceived(
                    suppliedTime
                );
            }
        );

        const TTick sourceTime = this->GetSourceTime();

        ApplySynchronizedTime(
            time,
            sourceTime,
            true
        );
    }

    /// <summary>Attempts to write the RTC and, on success, makes the written value the synchronized clock origin.</summary>
    bool TrySetTime(
        const TTime& time
    ) {
        const TTick attemptedTime =
            this->GetNanoseconds(time);

        bool writeSucceeded = false;

        {
            typename TLockPolicy::Guard ioLock(_rtcIOMutex);
            writeSucceeded = WriteRTC(time);
        }

        if (!writeSucceeded) {
            _observable->Notify<
                IRTCClockObserver<TTime, TTick>
            >(
                [&](IRTCClockObserver<TTime, TTick>* observer) {
                    observer->OnRTCTimeWriteFailed(
                        attemptedTime
                    );
                }
            );

            return false;
        }

        const TTick sourceTime = this->GetSourceTime();
        TTick previousTime = 0;

        {
            typename TLockPolicy::Guard lock(_stateMutex);

            previousTime =
                GetTimeNanosecondsLocked(sourceTime);

            _sourceTime = sourceTime;
            _rtcTime = attemptedTime;
            _isSynchronized = true;
        }

        _observable->Notify<
            IRTCClockObserver<TTime, TTick>
        >(
            [&](IRTCClockObserver<TTime, TTick>* observer) {
                observer->OnRTCTimeWriteSucceeded(
                    previousTime,
                    attemptedTime,
                    Internal::SignedDifference(
                        attemptedTime,
                        previousTime
                    )
                );
            }
        );

        return true;
    }

    /// <inheritdoc/>
    TTime GetTime() const override {
        const TTick sourceTime = this->GetSourceTime();
        typename TLockPolicy::Guard lock(_stateMutex);

        return this->CreateTime(
            GetTimeNanosecondsLocked(sourceTime),
            _rtcResolution
        );
    }

    /// <inheritdoc/>
    TTime GetResolution() const override {
        const TTick sourceResolution =
            static_cast<TTick>(
                Internal::GetSourceResolution(
                    this->_timeSource->GetTicksPerSecond()
                )
            );

        const TTick resolution =
            _rtcResolution > sourceResolution
                ? _rtcResolution
                : sourceResolution;

        return this->CreateTime(
            resolution,
            resolution
        );
    }

    /// <inheritdoc/>
    bool GetIsSynchronized() const override {
        typename TLockPolicy::Guard lock(_stateMutex);
        return _isSynchronized;
    }

    /// <inheritdoc/>
    void SetTime(
        const TTime& time
    ) override {
        TrySetTime(time);
    }

    /// <summary>Registers an observer for RTC clock lifecycle notifications.</summary>
    Observable::ObserverHandlePtr RegisterObserver(
        IRTCClockObserver<TTime, TTick>* observer
    ) {
        return _observable->RegisterObserver(observer);
    }

    /// <summary>Unregisters an RTC clock observer.</summary>
    void UnregisterObserver(
        IRTCClockObserver<TTime, TTick>* observer
    ) {
        _observable->UnregisterObserver(observer);
    }
};

/// <summary>RTC clock base specialization using a no-op lock policy for single-threaded consumers.</summary>
template<
    typename TTime = DefaultClockTime,
    typename TTick = ClockTick
>
using SingleThreadedRTCClockBase =
    RTCClockBase<TTime, NoLockPolicy, TTick>;

} // namespace Timing
} // namespace ESPressio
