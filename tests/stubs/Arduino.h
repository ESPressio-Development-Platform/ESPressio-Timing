#pragma once

#include <cstdint>
#include <string>

/**
 * ESPressio Memory Audit
 * Members:
 * - _value (std::string): 24 bytes [Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Total Memory: 24 bytes [_value: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class String {
    private:
        std::string _value;

    public:
        String(const char* value = "") : _value(value) { }

        String& operator+=(const char* value) {
            _value += value;
            return *this;
        }

        const char* c_str() const {
            return _value.c_str();
        }
};

inline uint32_t& ArduinoMicrosValue() {
    static uint32_t value = 0;
    return value;
}

inline uint32_t micros() {
    return ArduinoMicrosValue();
}
