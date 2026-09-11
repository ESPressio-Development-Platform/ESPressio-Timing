cmake_minimum_required(VERSION 3.16)

project(
    ESPressioTimingContracts
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(ESPRESSIO_SYSTEM_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../ESPressio-System" CACHE PATH "System coordinated source checkout")
include_directories(${ESPRESSIO_SYSTEM_SOURCE_DIR}/src)

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
    test_monotonic_clock
    test_monotonic_clock.cpp
)

target_include_directories(
    test_monotonic_clock
    PRIVATE
    ../src
    ../test_support
)

add_test(
    NAME MonotonicClock
    COMMAND test_monotonic_clock
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
    test_clock_sample_validation
    test_clock_sample_validation.cpp
)

target_include_directories(
    test_clock_sample_validation
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
    NAME ClockSampleValidation
    COMMAND test_clock_sample_validation
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

find_package(Threads REQUIRED)
foreach(test clock_qualification clock_affine_regression stopwatch_concurrency)
    add_executable(test_${test} test_${test}.cpp)
    target_include_directories(test_${test} PRIVATE ../src ../test_support)
    target_link_libraries(test_${test} PRIVATE Threads::Threads)
    add_test(NAME ${test} COMMAND test_${test})
endforeach()

foreach(test gptimer_compile gptimer_fallback arduino_compile)
    add_executable(test_${test} test_${test}.cpp)
    target_include_directories(test_${test} PRIVATE ../src ../test_support)
    add_test(NAME ${test} COMMAND test_${test})
endforeach()
target_compile_definitions(test_arduino_compile PRIVATE ARDUINO=1)
