#pragma once

#include <cstdint>

#include "ESPressio_ITimeSource.hpp"

#ifndef ESPRESSIO_TIMING_HAS_GPTIMER
    #if defined(ESP32) && defined(__has_include)
        #if __has_include("driver/gptimer.h")
            #define ESPRESSIO_TIMING_HAS_GPTIMER 1
        #else
            #define ESPRESSIO_TIMING_HAS_GPTIMER 0
        #endif
    #else
        #define ESPRESSIO_TIMING_HAS_GPTIMER 0
    #endif
#endif

#ifndef ESPRESSIO_TIMING_GPTIMER_DEFAULT_RESOLUTION_HZ
    #define ESPRESSIO_TIMING_GPTIMER_DEFAULT_RESOLUTION_HZ 10000000UL
#endif

#if ESPRESSIO_TIMING_HAS_GPTIMER

#include "driver/gptimer.h"
#include "esp_err.h"

namespace ESPressio {

    namespace Timing {

        class GPTimerTimeSource :
            public ITimeSource {

            private:
                gptimer_handle_t _timer = nullptr;
                uint32_t _resolution = 0;
                esp_err_t _initializationResult = ESP_OK;
                bool _isEnabled = false;
                bool _isRunning = false;

                void Initialize(
                    uint32_t requestedResolution
                ) {
                    if (requestedResolution == 0) {
                        _initializationResult =
                            ESP_ERR_INVALID_ARG;
                        return;
                    }

                    gptimer_config_t configuration = {};
                    configuration.clk_src =
                        GPTIMER_CLK_SRC_DEFAULT;
                    configuration.direction =
                        GPTIMER_COUNT_UP;
                    configuration.resolution_hz =
                        requestedResolution;

                    _initializationResult =
                        gptimer_new_timer(
                            &configuration,
                            &_timer
                        );

                    if (_initializationResult != ESP_OK) {
                        _timer = nullptr;
                        return;
                    }

                    _initializationResult =
                        gptimer_get_resolution(
                            _timer,
                            &_resolution
                        );

                    if (_initializationResult != ESP_OK) {
                        gptimer_del_timer(_timer);
                        _timer = nullptr;
                        _resolution = 0;
                        return;
                    }

                    _initializationResult =
                        gptimer_enable(_timer);

                    if (_initializationResult != ESP_OK) {
                        gptimer_del_timer(_timer);
                        _timer = nullptr;
                        _resolution = 0;
                        return;
                    }

                    _isEnabled = true;

                    _initializationResult =
                        gptimer_start(_timer);

                    if (_initializationResult != ESP_OK) {
                        gptimer_disable(_timer);
                        gptimer_del_timer(_timer);
                        _timer = nullptr;
                        _resolution = 0;
                        _isEnabled = false;
                        return;
                    }

                    _isRunning = true;
                }

            public:
                explicit GPTimerTimeSource(
                    uint32_t requestedResolution =
                        ESPRESSIO_TIMING_GPTIMER_DEFAULT_RESOLUTION_HZ
                ) {
                    Initialize(
                        requestedResolution
                    );
                }

                ~GPTimerTimeSource() override {
                    if (_timer == nullptr) {
                        return;
                    }

                    if (_isRunning) {
                        gptimer_stop(_timer);
                    }

                    if (_isEnabled) {
                        gptimer_disable(_timer);
                    }

                    gptimer_del_timer(_timer);
                }

                GPTimerTimeSource(
                    const GPTimerTimeSource&
                ) = delete;

                GPTimerTimeSource& operator=(
                    const GPTimerTimeSource&
                ) = delete;

                GPTimerTimeSource(
                    GPTimerTimeSource&&
                ) = delete;

                GPTimerTimeSource& operator=(
                    GPTimerTimeSource&&
                ) = delete;


                uint64_t GetTicks() const override {
                    if (!_isRunning) {
                        return 0;
                    }

                    uint64_t count = 0;

                    return
                        gptimer_get_raw_count(
                            _timer,
                            &count
                        ) == ESP_OK
                            ? count
                            : 0;
                }


                uint64_t GetTicksPerSecond() const override {
                    return _resolution;
                }


                bool GetIsAvailable() const {
                    return _isRunning;
                }


                esp_err_t GetInitializationResult() const {
                    return _initializationResult;
                }
        };

    }

}

#endif
