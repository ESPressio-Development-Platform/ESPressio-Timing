#pragma once
#include <ESPressio_SystemPlatformClock.hpp>
#include <memory>
struct CounterTestProvider final : ESPressio::System::Clock::IHighResolutionCounterProvider {
    using Result=ESPressio::System::PlatformResult;
    using Status=ESPressio::System::PlatformStatus;
    std::uint64_t Ticks=0;
    unsigned Created=0;
    bool FailCreate=false,FailStart=false;
    struct Counter final : ESPressio::System::Clock::IHighResolutionCounter {
        CounterTestProvider& Owner; std::uint64_t Hertz;
        Counter(CounterTestProvider& owner,std::uint64_t hertz):Owner(owner),Hertz(hertz) {}
        Result Start() noexcept override { return Owner.FailStart ? Result::Failed(Status::HardwareFailure) : Result::Succeeded(); }
        Result Stop() noexcept override { return Result::Succeeded(); }
        Result Reset() noexcept override { Owner.Ticks=0; return Result::Succeeded(); }
        Result Read(std::uint64_t& value) const noexcept override { value=Owner.Ticks; return Result::Succeeded(); }
        std::uint64_t ResolutionHz() const noexcept override { return Hertz; }
        bool IsAvailable() const noexcept override { return true; }
        bool IsInterruptSafe() const noexcept override { return true; }
        Result InitializationResult() const noexcept override { return Result::Succeeded(); }
    };
    std::unique_ptr<ESPressio::System::Clock::IHighResolutionCounter> Create(std::uint64_t hz) override {
        if (FailCreate) return {};
        ++Created; return std::make_unique<Counter>(*this,hz);
    }
};
struct TestMonotonic final : ESPressio::System::Clock::IMonotonicClock {
    std::uint64_t Now=0;
    std::uint64_t NowNanoseconds() const noexcept override { return Now; }
    std::uint64_t ResolutionNanoseconds() const noexcept override { return 1000; }
    bool IsInterruptSafe() const noexcept override { return false; }
};
