#pragma once

#include <cstdint>
#include <memory>

#include <ESPressio_SystemPlatformClock.hpp>

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

    /// <summary>Compatibility time-source adapter exposing the historical GPTimer API over the platform-neutral System high-resolution counter.</summary>
    /// <remarks>New code should prefer <c>HighResolutionTimeSource</c>; this adapter no longer owns or directly references ESP-IDF GPTimer resources.</remarks>
    class GPTimerTimeSource : public ITimeSource {
    private:
        std::unique_ptr<System::Clock::IHighResolutionCounter> _counter;
        System::PlatformResult _initializationResult =
            System::PlatformResult::Failed(System::PlatformStatus::Unavailable);

    public:
        /// <summary>Creates and starts a high-resolution counter at the requested resolution when supported by the platform provider.</summary>
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

        /// <inheritdoc/>
        uint64_t GetTicks() const override {
            if (_counter == nullptr || !_counter->IsAvailable()) return 0;
            uint64_t count = 0;
            return _counter->Read(count) ? count : 0;
        }

        /// <inheritdoc/>
        uint64_t GetTicksPerSecond() const override {
            return _counter != nullptr ? _counter->ResolutionHz() : 0;
        }

        /// <summary>Indicates whether the platform counter is available and ready for reads.</summary>
        bool GetIsAvailable() const {
            return _counter != nullptr && _counter->IsAvailable();
        }

        /// <summary>Returns the result of counter creation/initialization/startup.</summary>
        System::PlatformResult GetInitializationResult() const {
            return _initializationResult;
        }
    };

}
}
