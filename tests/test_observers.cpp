#include <cassert>
#include <cstdint>
#include <stdexcept>

#include <ESPressio_Timing.hpp>
#include "ClockTestEvidence.hpp"

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
        const ClockSynchronizationResult&, const ClockSynchronizationStatus&) override {
        ++acceptedCount; before=previous; after=next; diff=difference;
    }
    void OnSystemClockSynchronized(uint64_t previous, uint64_t next, int64_t difference,
        const ClockSynchronizationResult&, const ClockSynchronizationStatus&) override {
        ++synchronizedCount; before=previous; after=next; diff=difference;
    }
    void OnSystemClockSynchronizationSampleRejected(const ClockSynchronizationResult&,
        const ClockSynchronizationStatus&) override { ++rejectedCount; }
    void OnSystemClockSynchronizationStateChanged(TimeReliability, TimeReliability,
        const ClockSynchronizationStatus&) override { ++stateChangedCount; }
    void OnSystemClockSynchronizationReset(const ClockSynchronizationStatus&,
        const ClockSynchronizationStatus&) override { ++resetCount; }
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

    clock.TrySetTime(DefaultClockTime(1000, Units::Nano));
    assert(systemObserver.setCount == 1);
    assert(systemObserver.after == 1000);

    ClockSynchronizationProfile cfg;
    assert(clock.ConfigureSynchronization(cfg)==ClockConfigurationStatus::Success);
    assert(clock.SelectSynchronizationReference(1)==ClockConfigurationStatus::Success);
    clock.SealContinuity();
    for (unsigned i=0;i<4;++i) {
        source.ticks=1000000000ull+i*250000000ull;
        const auto model=clock.GetClockModelSnapshot();
        const auto sample=ClockTest::Observation(source.ticks,1000,10000,1,100,&model);
        source.ticks+=10000;
        assert(clock.SubmitSynchronizationObservation(sample).Accepted);
    }
    assert(systemObserver.acceptedCount==4 && systemObserver.synchronizedCount==1);
    assert(systemObserver.diff==0); // Synchronization never steps, including before maturity.
    auto bad=ClockTest::Observation(source.ticks+1000000);
    bad.T1.Quality=ClockCaptureQuality::Invalid;
    assert(!clock.SubmitSynchronizationObservation(bad).Accepted);
    assert(systemObserver.rejectedCount==1);
    const int changes=systemObserver.stateChangedCount;
    source.ticks+=30000000000ull;
    for (unsigned i=0;i<10;++i) {
        (void)clock.GetTime(); (void)clock.CaptureQualifiedTime(); (void)clock.GetSynchronizationStatus();
    }
    assert(systemObserver.stateChangedCount==changes); // Even qualification expiry cannot notify from reads.
    clock.Update(); assert(systemObserver.stateChangedCount==changes+1);
    bool callbackRan=false;
    auto now=clock.CaptureQualifiedTime().Nanoseconds;
    assert(clock.TrySetCallback(DefaultClockTime(now,Units::Nano),[&]{callbackRan=true;}));
    clock.Update(); assert(callbackRan && systemObserver.callbackExecutedCount==1);
    assert(clock.TrySetCallback(DefaultClockTime(now,Units::Nano),[]{throw std::runtime_error("boom");}));
    bool caught=false; try { clock.Update(); } catch (const std::runtime_error&) { caught=true; }
    assert(caught && systemObserver.callbackFailedCount==1);

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
