#include "pipeline/DataValidator.h"
#include "core/Logger.h"

namespace iot {

DataValidator::DataValidator(double minValid, double maxValid)
    : minValid_(minValid), maxValid_(maxValid) {}

SensorReading DataValidator::handle(SensorReading reading) {
    auto& log = Logger::getInstance();

    if (!reading.isValid) {
        log.warning("Validator: already invalid reading from " +
                    reading.sensorName, "DataValidator");
        return reading;  // Don't forward invalid readings
    }

    if (reading.processedValue < minValid_ ||
        reading.processedValue > maxValid_) {
        log.warning(
            "Validator: out-of-range value " +
            std::to_string(reading.processedValue) +
            " from " + reading.sensorName +
            " [" + std::to_string(minValid_) + ", " +
            std::to_string(maxValid_) + "]",
            "DataValidator");
        reading.isValid = false;
    } else {
        log.debug(
            "Validator: PASS " + reading.sensorName +
            " value=" + std::to_string(reading.processedValue),
            "DataValidator");
    }

    return BaseDataHandler::handle(std::move(reading));
}

std::string DataValidator::getHandlerName() const {
    return "DataValidator";
}

} // namespace iot
