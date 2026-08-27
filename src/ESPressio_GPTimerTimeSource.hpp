#pragma once

#include <cstdint>
#include <memory>

#include <ESPressio_SystemClock.hpp>

#include "ESPressio_ITimeSource.hpp"

#ifndef ESPRESSIO_TIMING_GPTIMER_DEFAULT_RESOLUTION_HZ
    #define ESPRESSIO_TIMING_GPTIMER_DEFAULT_RESOLUTION_HZ 10000000UL
#endif

#ifndef ESPRESSIO_TIMING_HAS_GPTIMER
    // Compatibility macro retained for existing consumers. The implementation
    // is now provider-driven and therefore no longer depends on an ESP32
    // compile-time capability check.
    #define ESPRESSIO_TIMING_HAS_GPTIMER 1
#endif

namespace ESPressio {
namespace Timing {

    /*
     * Compatibility adapter for the historical GPTimerTimeSource API.
     * The class no longer owns or references ESP-IDF GPTimer resources; it
     * consumes the platform-neutral System high-resolution counter capability.
     * New code should prefer HighResolutionTimeSource.
     */
    class GPTimerTimeSource : public ITimeSource {
    private:
        std::unique_ptr<System::Clock::IHighResolutionCounter> _counter;
        System::PlatformResult _initializationResult =
            System::PlatformResult::Failed(System::PlatformStatus::Unavailable);

    public:
        explicit GPTimerTimeSource(
            uint32_t requestedResolution =
                ESPRESSIO_TIMING_GPTIMER_DEFAULT_RESOLUTION_HZ
        ) {
            if (requestedResolution == 0) {
                _initializationResult = System::PlatformResult::Failed(
                    System::PlatformStatus::InvalidArgument
                );
                return;
            }

            _counter = System::Clock::CreateHighResolutionCounter(
                requestedResolution
            );
            if (_counter == nullptr) {
                return;
            }

            _initializationResult = _counter->InitializationResult();
            if (!_initializationResult || !_counter->IsAvailable()) {
                _counter.reset();
                return;
            }

            _initializationResult = _counter->Start();
            if (!_initializationResult) {
                _counter.reset();
            }
        }

        ~GPTimerTimeSource() override = default;

        GPTimerTimeSource(const GPTimerTimeSource&) = delete;
        GPTimerTimeSource& operator=(const GPTimerTimeSource&) = delete;
        GPTimerTimeSource(GPTimerTimeSource&&) = delete;
        GPTimerTimeSource& operator=(GPTimerTimeSource&&) = delete;

        uint64_t GetTicks() const override {
            if (_counter == nullptr || !_counter->IsAvailable()) return 0;
            uint64_t count = 0;
            return _counter->Read(count) ? count : 0;
        }

        uint64_t GetTicksPerSecond() const override {
            return _counter != nullptr ? _counter->ResolutionHz() : 0;
        }

        bool GetIsAvailable() const {
            return _counter != nullptr && _counter->IsAvailable();
        }

        System::PlatformResult GetInitializationResult() const {
            return _initializationResult;
        }
    };

}
}
