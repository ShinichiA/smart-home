#pragma once

#include <string>
#include <cstdint>

namespace iot {

// ─── Sensor Type Enum ────────────────────────────────────────────────
enum class SensorType : uint8_t {
    Temperature = 0,
    Humidity    = 1,
    Motion      = 2
};

/**
 * @brief Convert SensorType enum to string.
 */
[[nodiscard]] inline std::string sensorTypeToString(SensorType type) {
    switch (type) {
        case SensorType::Temperature: return "Temperature";
        case SensorType::Humidity:    return "Humidity";
        case SensorType::Motion:      return "Motion";
    }
    return "Unknown";
}

} // namespace iot
