#pragma once

#include "sensors/interfaces/SensorReading.h"
#include <string>

namespace iot {

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

protected:
    // Protected to prevent slicing while allowing subclass construction
    ISensor() = default;
    ISensor(const ISensor&) = default;
    ISensor& operator=(const ISensor&) = default;
    ISensor(ISensor&&) = default;
    ISensor& operator=(ISensor&&) = default;
};

} // namespace iot
