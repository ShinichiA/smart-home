#pragma once

#include "sensors/interfaces/ISensor.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

namespace iot {

/**
 * @brief Factory Method pattern for creating sensor instances.
 *
 * Design Patterns: Factory Method
 * Modern C++: std::unique_ptr, std::function, unordered_map,
 *             move semantics, auto type deduction
 *
 * Supports runtime registration of custom sensor creators.
 */
class SensorFactory {
public:
    using SensorCreator = std::function<std::unique_ptr<ISensor>(
        const std::string& name, int pin)>;

    /**
     * @brief Create a sensor by type enum.
     */
    static std::unique_ptr<ISensor> create(SensorType type,
                                            const std::string& name,
                                            int pin);

    /**
     * @brief Create a sensor by type string.
     */
    static std::unique_ptr<ISensor> create(const std::string& typeStr,
                                            const std::string& name,
                                            int pin);

    /**
     * @brief Register a custom sensor creator for extensibility.
     */
    static void registerCreator(const std::string& typeStr,
                                SensorCreator creator);

private:
    static std::unordered_map<std::string, SensorCreator>& getRegistry();
};

} // namespace iot
