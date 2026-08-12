cmake_minimum_required(VERSION 3.16)
project(ESPressioTimingTests LANGUAGES CXX)

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

target_include_directories(
    espressio_timing_tests
    PRIVATE
    ../src
    ../../ESPressio-Units/src
    ../../ESPressio-Units/tests/stubs
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

enable_testing()
add_test(NAME espressio_timing_tests COMMAND espressio_timing_tests)
add_test(
    NAME espressio_timing_arduino_compile
    COMMAND espressio_timing_arduino_compile
)
