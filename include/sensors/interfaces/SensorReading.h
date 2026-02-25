#pragma once

#include "sensors/interfaces/SensorTypes.h"
#include <string>
#include <cstdint>
#include <optional>

namespace iot {

/**
 * @brief Sensor reading data structure.
 * Modern C++: aggregate initialization, std::optional
 */
struct SensorReading {
    std::string sensorName;
    SensorType  type;
    double      rawValue;
    double      processedValue;
    uint64_t    timestampMs;
    bool        isValid{true};
    std::optional<std::string> unit;
};

} // namespace iot
