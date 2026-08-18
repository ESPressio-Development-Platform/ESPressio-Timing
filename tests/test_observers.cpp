#include <cassert>
#include <cstdint>
#include <stdexcept>

#include <ESPressio_Timing.hpp>

using namespace ESPressio;
using namespace ESPressio::Timing;

class ObserverFakeSource : public ITimeSource {
public:
    uint64_t ticks = 0;
    uint64_t GetTicks() const override { return ticks; }
    uint64_t GetTicksPerSecond() const override { return 1000000000ULL; }
};

class SystemObserver : public ISystemClockObserver<uint64_t> {
public:
    int setCount = 0;
    int acceptedCount = 0;
    int rejectedCount = 0;
    int synchronizedCount = 0;
    int stateChangedCount = 0;
    int callbackScheduledCount = 0;
    int callbackExecutedCount = 0;
    int callbackFailedCount = 0;
    int resetCount = 0;

    uint64_t before = 0;
    uint64_t after = 0;
    int64_t diff = 0;

    void OnSystemClockTimeSet(uint64_t previous, uint64_t next, int64_t difference) override {
        ++setCount; before=previous; after=next; diff=difference;
    }
    void OnSystemClockSynchronizationSampleAccepted(uint64_t previous, uint64_t next, int64_t difference,
        const ClockSynchronizationResult<uint64_t>&, const ClockSynchronizationStatus<uint64_t>&) override {
        ++acceptedCount; before=previous; after=next; diff=difference;
    }
    void OnSystemClockSynchronized(uint64_t previous, uint64_t next, int64_t difference,
        const ClockSynchronizationResult<uint64_t>&, const ClockSynchronizationStatus<uint64_t>&) override {
        ++synchronizedCount; before=previous; after=next; diff=difference;
    }
    void OnSystemClockSynchronizationSampleRejected(const ClockSynchronizationResult<uint64_t>&,
        const ClockSynchronizationStatus<uint64_t>&) override { ++rejectedCount; }
    void OnSystemClockSynchronizationStateChanged(ClockSynchronizationState, ClockSynchronizationState,
        const ClockSynchronizationStatus<uint64_t>&) override { ++stateChangedCount; }
    void OnSystemClockSynchronizationReset(const ClockSynchronizationStatus<uint64_t>&,
        const ClockSynchronizationStatus<uint64_t>&) override { ++resetCount; }
    void OnSystemClockCallbackScheduled(uint64_t) override { ++callbackScheduledCount; }
    void OnSystemClockCallbackExecuted(uint64_t, uint64_t, int64_t) override { ++callbackExecutedCount; }
    void OnSystemClockCallbackExecutionFailed(uint64_t, uint64_t, int64_t, std::exception_ptr) override { ++callbackFailedCount; }
};

class StopwatchObserver : public IStopwatchClockObserver<DefaultClockTime, uint64_t> {
public:
    int starts=0, stops=0, resets=0, restarts=0, sets=0;
    void OnStopwatchStarted(uint64_t) override { ++starts; }
    void OnStopwatchStopped(uint64_t) override { ++stops; }
    void OnStopwatchReset(uint64_t, bool) override { ++resets; }
    void OnStopwatchRestarted(uint64_t) override { ++restarts; }
    void OnStopwatchTimeSet(uint64_t, uint64_t, int64_t, bool) override { ++sets; }
};

class ObserverRTC : public RTCClockBase<DefaultClockTime, NoLockPolicy> {
public:
    bool readSucceeds=true;
    bool writeSucceeds=true;
    DefaultClockTime stored{10, Units::Base};
    explicit ObserverRTC(ObserverFakeSource* s) : RTCClockBase(DefaultClockTime(1, Units::Nano), s) {}
protected:
    bool ReadRTC(DefaultClockTime& t) override { if(!readSucceeds) return false; t=stored; return true; }
    bool WriteRTC(const DefaultClockTime& t) override { if(!writeSucceeds) return false; stored=t; return true; }
};

class RTCObserver : public IRTCClockObserver<DefaultClockTime, uint64_t> {
public:
    int syncSuccess=0, syncFail=0, interrupts=0, timeInterrupts=0, writeSuccess=0, writeFail=0;
    void OnRTCSynchronizationSucceeded(uint64_t, uint64_t, int64_t) override { ++syncSuccess; }
    void OnRTCSynchronizationFailed() override { ++syncFail; }
    void OnRTCInterruptReceived() override { ++interrupts; }
    void OnRTCInterruptTimeReceived(uint64_t) override { ++timeInterrupts; }
    void OnRTCTimeWriteSucceeded(uint64_t, uint64_t, int64_t) override { ++writeSuccess; }
    void OnRTCTimeWriteFailed(uint64_t) override { ++writeFail; }
};

int main() {
    ObserverFakeSource source;

    using Clock = SystemClock<DefaultClockTime, NoLockPolicy>;
    auto& clock = Clock::GetInstance(&source);
    clock.ResetSynchronization();

    SystemObserver systemObserver;
    auto systemHandle = clock.RegisterObserver(&systemObserver);

    clock.SetTime(DefaultClockTime(1000, Units::Nano));
    assert(systemObserver.setCount == 1);
    assert(systemObserver.after == 1000);

    ClockSynchronizationConfig cfg;
    cfg.OffsetFilterWeight = 1.0;
    cfg.MinimumSamplesForSynchronizedState = 1;
    clock.ConfigureSynchronization(cfg);

    const uint64_t t1 = clock.GetSynchronizationTimestampNanoseconds();
    ClockSynchronizationSample<uint64_t> sample;
    sample.LocalRequestTransmitTime=t1;
    sample.RemoteRequestReceiveTime=t1+5000;
    sample.RemoteResponseTransmitTime=t1+5000;
    sample.LocalResponseReceiveTime=t1;

    auto result = clock.SubmitSynchronizationSample(sample, ClockSynchronizationAdjustmentMode::StepIfUnsynchronized);
    assert(result.Accepted);
    assert(systemObserver.acceptedCount == 1);
    assert(systemObserver.synchronizedCount == 1);
    assert(systemObserver.after - systemObserver.before == 5000);
    assert(systemObserver.diff == 5000);
    assert(systemObserver.stateChangedCount >= 1);

    ClockSynchronizationSample<uint64_t> bad = sample;
    bad.RemoteResponseTransmitTime = bad.RemoteRequestReceiveTime - 1;
    bad.LocalResponseReceiveTime = bad.LocalRequestTransmitTime;
    auto rejected = clock.SubmitSynchronizationSample(bad);
    assert(!rejected.Accepted);
    assert(systemObserver.rejectedCount == 1);

    bool callbackRan=false;
    const auto now = clock.GetSynchronizationTimestampNanoseconds();
    assert(clock.TrySetCallback(DefaultClockTime(now, Units::Nano), [&]{ callbackRan=true; }));
    assert(systemObserver.callbackScheduledCount == 1);
    clock.Update();
    assert(callbackRan);
    assert(systemObserver.callbackExecutedCount == 1);

    bool caught=false;
    const auto now2 = clock.GetSynchronizationTimestampNanoseconds();
    assert(clock.TrySetCallback(DefaultClockTime(now2, Units::Nano), []{ throw std::runtime_error("boom"); }));
    try { clock.Update(); } catch (const std::runtime_error&) { caught=true; }
    assert(caught);
    assert(systemObserver.callbackFailedCount == 1);

    /*
     * A slew can reach Synchronized later as the clock advances. A normal
     * GetTime() remains silent except for that genuine state transition.
     */
    clock.ResetSynchronization();
    cfg.MaximumSlewRatePpm = 1000000; // settle quickly in this host test
    cfg.SynchronizationToleranceNanoseconds = 1;
    clock.ConfigureSynchronization(cfg);

    const int stateChangesBeforeSlew = systemObserver.stateChangedCount;
    const uint64_t slewT1 = clock.GetSynchronizationTimestampNanoseconds();
    ClockSynchronizationSample<uint64_t> slewSample;
    slewSample.LocalRequestTransmitTime = slewT1;
    slewSample.RemoteRequestReceiveTime = slewT1 + 5000;
    slewSample.RemoteResponseTransmitTime = slewT1 + 5000;
    slewSample.LocalResponseReceiveTime = slewT1;
    assert(clock.SubmitSynchronizationSample(slewSample).Accepted);
    source.ticks += 10000;
    (void)clock.GetTime();
    assert(systemObserver.stateChangedCount > stateChangesBeforeSlew);

    StopwatchClock<DefaultClockTime, NoLockPolicy> stopwatch(false, &source);
    StopwatchObserver stopwatchObserver;
    auto stopwatchHandle = stopwatch.RegisterObserver(&stopwatchObserver);
    stopwatch.Start();
    stopwatch.Stop();
    stopwatch.Reset();
    stopwatch.Restart();
    stopwatch.SetTime(DefaultClockTime(42, Units::Nano));
    assert(stopwatchObserver.starts==1 && stopwatchObserver.stops==1 && stopwatchObserver.resets==1 && stopwatchObserver.restarts==1 && stopwatchObserver.sets==1);

    ObserverRTC rtc(&source);
    RTCObserver rtcObserver;
    auto rtcHandle = rtc.RegisterObserver(&rtcObserver);
    assert(rtc.Synchronize());
    assert(rtcObserver.syncSuccess==1);
    rtc.readSucceeds=false;
    assert(!rtc.Synchronize());
    assert(rtcObserver.syncFail==1);
    rtc.OnRTCInterrupt(DefaultClockTime(20, Units::Nano));
    assert(rtcObserver.timeInterrupts==1);
    assert(rtcObserver.syncSuccess==2);
    rtc.writeSucceeds=true;
    assert(rtc.TrySetTime(DefaultClockTime(30, Units::Nano)));
    assert(rtcObserver.writeSuccess==1);
    rtc.writeSucceeds=false;
    assert(!rtc.TrySetTime(DefaultClockTime(40, Units::Nano)));
    assert(rtcObserver.writeFail==1);

    return 0;
}
