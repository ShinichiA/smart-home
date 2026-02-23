#pragma once

#include <string>
#include <cstdint>
#include <optional>

namespace iot {

// ─── Sensor Type Enum ────────────────────────────────────────────────
enum class SensorType : uint8_t {
    Temperature = 0,
    Humidity    = 1,
    Motion      = 2
};

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

/**
 * @brief Pure virtual interface for all sensors (Interface Segregation).
 *
 * Follows SOLID: Interface Segregation Principle.
 * Modern C++: pure virtual, default virtual destructor, enum class,
 *             nodiscard attribute
 */
class ISensor {
public:
    virtual ~ISensor() = default;

    [[nodiscard]] virtual std::string   getName() const = 0;
    [[nodiscard]] virtual SensorType    getType() const = 0;
    [[nodiscard]] virtual bool          isInitialized() const = 0;

    virtual bool            initialize() = 0;
    virtual void            shutdown() = 0;
    virtual SensorReading   read() = 0;
    virtual void            calibrate(double offset) = 0;

    // Prevent slicing
    ISensor() = default;
    ISensor(const ISensor&) = default;
    ISensor& operator=(const ISensor&) = default;
    ISensor(ISensor&&) = default;
    ISensor& operator=(ISensor&&) = default;
};

/**
 * @brief Convert SensorType enum to string.
 */
inline std::string sensorTypeToString(SensorType type) {
    switch (type) {
        case SensorType::Temperature: return "Temperature";
        case SensorType::Humidity:    return "Humidity";
        case SensorType::Motion:      return "Motion";
    }
    return "Unknown";
}

} // namespace iot
