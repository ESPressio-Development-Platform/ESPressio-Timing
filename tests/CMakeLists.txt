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


add_executable(
    test_clock_synchronization
    test_clock_synchronization.cpp
)

target_include_directories(
    test_clock_synchronization
    PRIVATE
    ../src
    ../test_support
)

add_executable(
    test_synchronized_system_clock
    test_synchronized_system_clock.cpp
)

target_include_directories(
    test_synchronized_system_clock
    PRIVATE
    ../src
    ../test_support
)

add_test(
    NAME ClockSynchronization
    COMMAND test_clock_synchronization
)

add_test(
    NAME SynchronizedSystemClock
    COMMAND test_synchronized_system_clock
)


add_executable(
    test_observers
    test_observers.cpp
)

target_include_directories(
    test_observers
    PRIVATE
    ../src
    ../test_support
)

add_test(
    NAME Observers
    COMMAND test_observers
)
