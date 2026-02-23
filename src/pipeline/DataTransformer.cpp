#include "pipeline/DataTransformer.h"
#include "core/Logger.h"

namespace iot {

SensorReading DataTransformer::handle(SensorReading reading) {
    if (!reading.isValid) {
        return BaseDataHandler::handle(std::move(reading));
    }

    auto it = transforms_.find(reading.type);
    if (it != transforms_.end()) {
        double original = reading.processedValue;
        reading.processedValue = it->second(original);

        Logger::getInstance().debug(
            "Transform [" + reading.sensorName + "]: " +
            std::to_string(original) + " -> " +
            std::to_string(reading.processedValue),
            "DataTransformer");
    }

    return BaseDataHandler::handle(std::move(reading));
}

std::string DataTransformer::getHandlerName() const {
    return "DataTransformer";
}

void DataTransformer::addTransform(SensorType type,
                                    TransformFunc transform) {
    transforms_[type] = std::move(transform);
    Logger::getInstance().info(
        "Registered transform for: " + sensorTypeToString(type),
        "DataTransformer");
}

} // namespace iot
