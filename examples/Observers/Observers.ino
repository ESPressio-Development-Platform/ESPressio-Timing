#include <Arduino.h>
#include <ESPressio_TimingSystemClock.hpp>
using namespace ESPressio;
class ClockObserver final : public Timing::ISystemClockObserver<> {
public:
    void OnSystemClockSynchronizationStateChanged(Timing::TimeReliability,Timing::TimeReliability reliability,
                                                 const Timing::ClockSynchronizationStatus& status) override {
        Serial.printf("reliability=%u known=%u uncertainty=%llu ns\n",static_cast<unsigned>(reliability),
            status.CurrentUncertainty.IsKnown,static_cast<unsigned long long>(status.CurrentUncertainty.Nanoseconds));
    }
};
ClockObserver observer;
Observable::ObserverHandlePtr registration;
void setup() {
    Serial.begin(115200);
    registration=Timing::SystemClock<>::GetInstance().RegisterObserver(&observer);
}
void loop() {
    // Explicit service may notify; GetTime/GetSynchronizationStatus never invoke observers.
    Timing::SystemClock<>::GetInstance().Update();
    delay(100);
}
