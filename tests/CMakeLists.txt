cmake_minimum_required(VERSION 3.16)
project(ESPressioTimingTests LANGUAGES CXX)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
    espressio_timing_tests
    test_timing.cpp
)

add_executable(
    espressio_timing_arduino_compile
    test_arduino_compile.cpp
)

add_executable(
    espressio_timing_gptimer_compile
    test_gptimer_compile.cpp
)

add_executable(
    espressio_timing_gptimer_fallback
    test_gptimer_fallback.cpp
)

target_include_directories(
    espressio_timing_tests
    PRIVATE
    ../src
    ../../ESPressio-Units/src
    ../../ESPressio-Units/tests/stubs
)

target_link_libraries(
    espressio_timing_tests
    PRIVATE
    Threads::Threads
)

target_include_directories(
    espressio_timing_arduino_compile
    PRIVATE
    stubs
    ../src
    ../../ESPressio-Units/src
)

target_compile_definitions(
    espressio_timing_arduino_compile
    PRIVATE
    ARDUINO
)

target_include_directories(
    espressio_timing_gptimer_compile
    PRIVATE
    stubs
    ../src
    ../../ESPressio-Units/src
)

target_compile_definitions(
    espressio_timing_gptimer_compile
    PRIVATE
    ARDUINO
    ESP32
)

target_include_directories(
    espressio_timing_gptimer_fallback
    PRIVATE
    stubs
    ../src
    ../../ESPressio-Units/src
)

target_compile_definitions(
    espressio_timing_gptimer_fallback
    PRIVATE
    ARDUINO
    ESP32
    GPTIMER_STUB_FAIL
)

enable_testing()
add_test(NAME espressio_timing_tests COMMAND espressio_timing_tests)
add_test(
    NAME espressio_timing_arduino_compile
    COMMAND espressio_timing_arduino_compile
)
add_test(
    NAME espressio_timing_gptimer_compile
    COMMAND espressio_timing_gptimer_compile
)
add_test(
    NAME espressio_timing_gptimer_fallback
    COMMAND espressio_timing_gptimer_fallback
)
