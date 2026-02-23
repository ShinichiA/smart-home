#include "sensors/MotionSensor.h"

namespace iot {

MotionSensor::MotionSensor(const std::string& name, int pin,
                             double sensitivity)
    : BaseSensor(name, SensorType::Motion, pin)
    , sensitivity_(sensitivity)
{}

bool MotionSensor::onInitialize() {
    Logger::getInstance().debug(
        "Motion sensor sensitivity: " + std::to_string(sensitivity_),
        getName());
    return true;
}

double MotionSensor::readRawValue() {
    // Simulate: random chance of motion detection
    double chance = generateRandomValue(0.0, 1.0);
    return (chance > sensitivity_) ? 1.0 : 0.0;
}

bool MotionSensor::validateReading(double value) const {
    return value == 0.0 || value == 1.0;
}

std::string MotionSensor::getUnit() const {
    return "bool";
}

} // namespace iot
