#include "sensors/TemperatureSensor.h"

namespace iot {

TemperatureSensor::TemperatureSensor(const std::string& name, int pin,
                                     double minTemp, double maxTemp)
    : BaseSensor(name, SensorType::Temperature, pin)
    , minTemp_(minTemp)
    , maxTemp_(maxTemp)
{}

bool TemperatureSensor::onInitialize() {
    Logger::getInstance().debug(
        "Temperature sensor range: [" + std::to_string(minTemp_) +
        ", " + std::to_string(maxTemp_) + "] °C", getName());
    return true;
}

double TemperatureSensor::readRawValue() {
    // Simulate realistic temperature drift
    double drift = generateRandomValue(-0.5, 0.5);
    lastReading_ += drift;

    // Clamp to valid range
    if (lastReading_ < minTemp_) lastReading_ = minTemp_ + 1.0;
    if (lastReading_ > maxTemp_) lastReading_ = maxTemp_ - 1.0;

    return lastReading_;
}

double TemperatureSensor::applyCalibration(double raw) const {
    // Factory calibration + user offset
    return raw + calibrationOffset_ * 0.95; // sensor-specific gain factor
}

bool TemperatureSensor::validateReading(double value) const {
    return value >= minTemp_ && value <= maxTemp_;
}

std::string TemperatureSensor::getUnit() const {
    return "°C";
}

} // namespace iot
