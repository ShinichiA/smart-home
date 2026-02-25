#include "sensors/SensorFactory.h"
#include "sensors/TemperatureSensor.h"
#include "sensors/HumiditySensor.h"
#include "sensors/MotionSensor.h"
#include "core/Logger.h"

namespace iot {

std::unordered_map<std::string, SensorFactory::SensorCreator>&
SensorFactory::getRegistry() {
    static std::unordered_map<std::string, SensorCreator> registry;
    return registry;
}

std::unique_ptr<ISensor> SensorFactory::create(SensorType type,
                                                const std::string& name,
                                                int pin) {
    switch (type) {
        case SensorType::Temperature:
            return std::make_unique<TemperatureSensor>(name, pin);
        case SensorType::Humidity:
            return std::make_unique<HumiditySensor>(name, pin);
        case SensorType::Motion:
            return std::make_unique<MotionSensor>(name, pin);
    }

    Logger::getInstance().error(
        "Unknown SensorType: " +
        std::to_string(static_cast<int>(type)), "SensorFactory");
    return nullptr;
}

std::unique_ptr<ISensor> SensorFactory::create(const std::string& typeStr,
                                                const std::string& name,
                                                int pin) {
    // Check built-in types first
    if (typeStr == "temperature") return create(SensorType::Temperature, name, pin);
    if (typeStr == "humidity")    return create(SensorType::Humidity, name, pin);
    if (typeStr == "motion")      return create(SensorType::Motion, name, pin);

    // Check custom registry
    auto& registry = getRegistry();
    auto it = registry.find(typeStr);
    if (it != registry.end()) {
        Logger::getInstance().info(
            "Creating custom sensor type: " + typeStr, "SensorFactory");
        return it->second(name, pin);
    }

    Logger::getInstance().error(
        "Unknown sensor type string: " + typeStr, "SensorFactory");
    return nullptr;
}

void SensorFactory::registerCreator(const std::string& typeStr,
                                     SensorCreator creator) {
    getRegistry()[typeStr] = std::move(creator);
    Logger::getInstance().info(
        "Registered custom sensor type: " + typeStr, "SensorFactory");
}

} // namespace iot
