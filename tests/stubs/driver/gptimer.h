#pragma once

#include <cstdint>
#include "esp_err.h"

/**
 * ESPressio Memory Audit
 * Members:
 * - resolution (uint32_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 4 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
struct FakeGPTimer {
    uint32_t resolution;
};

typedef FakeGPTimer* gptimer_handle_t;

/**
 * ESPressio Memory Audit
 * Underlying storage: 4 bytes
 * Total Memory: 4 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
enum gptimer_clock_source_t {
    GPTIMER_CLK_SRC_DEFAULT
};

/**
 * ESPressio Memory Audit
 * Underlying storage: 4 bytes
 * Total Memory: 4 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
enum gptimer_count_direction_t {
    GPTIMER_COUNT_UP
};

/**
 * ESPressio Memory Audit
 * Members:
 * - clk_src (gptimer_clock_source_t): 4 bytes [0 bytes dynamic allocation]
 * - direction (gptimer_count_direction_t): 4 bytes [0 bytes dynamic allocation]
 * - resolution_hz (uint32_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 12 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
struct gptimer_config_t {
    gptimer_clock_source_t clk_src;
    gptimer_count_direction_t direction;
    uint32_t resolution_hz;
};

inline esp_err_t gptimer_new_timer(
    const gptimer_config_t* configuration,
    gptimer_handle_t* timer
) {
    #if defined(GPTIMER_STUB_FAIL)
        (void)configuration;
        (void)timer;
        return ESP_FAIL;
    #else
    static FakeGPTimer instance;
    instance.resolution = configuration->resolution_hz;
    *timer = &instance;
    return ESP_OK;
    #endif
}

inline esp_err_t gptimer_get_resolution(
    gptimer_handle_t timer,
    uint32_t* resolution
) {
    *resolution = timer->resolution;
    return ESP_OK;
}

inline esp_err_t gptimer_enable(gptimer_handle_t) {
    return ESP_OK;
}

inline esp_err_t gptimer_start(gptimer_handle_t) {
    return ESP_OK;
}

inline esp_err_t gptimer_get_raw_count(
    gptimer_handle_t,
    uint64_t* count
) {
    *count = 125;
    return ESP_OK;
}

inline esp_err_t gptimer_stop(gptimer_handle_t) {
    return ESP_OK;
}

inline esp_err_t gptimer_disable(gptimer_handle_t) {
    return ESP_OK;
}

inline esp_err_t gptimer_del_timer(gptimer_handle_t) {
    return ESP_OK;
}
