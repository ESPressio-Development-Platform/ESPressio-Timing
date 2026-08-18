cmake_minimum_required(VERSION 3.16)

project(
    ESPressioTimingV2Tests
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
    test_generic_timing
    test_generic_timing.cpp
)

target_include_directories(
    test_generic_timing
    PRIVATE
    ../src
    ../test_support
)

add_executable(
    test_rtc
    test_rtc.cpp
)

target_include_directories(
    test_rtc
    PRIVATE
    ../src
    ../test_support
)

enable_testing()

add_test(
    NAME GenericTiming
    COMMAND test_generic_timing
)

add_test(
    NAME RTC
    COMMAND test_rtc
)


add_executable(
    test_system_clock_shared_core
    test_system_clock_shared_core.cpp
)

target_include_directories(
    test_system_clock_shared_core
    PRIVATE
    ../src
    ../test_support
)

add_test(
    NAME SystemClockSharedCore
    COMMAND test_system_clock_shared_core
)
