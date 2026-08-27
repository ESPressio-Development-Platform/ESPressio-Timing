#pragma once

// Canonical unambiguous include for ESPressio::Timing::SystemClock.
//
// ESPressio-System also owns a historical ESPressio_SystemClock.hpp
// compatibility header. New cross-library code should include this file when
// it requires the disciplined Timing::SystemClock implementation so include
// resolution never depends on library search order.
#include "ESPressio_SystemClock.hpp"
