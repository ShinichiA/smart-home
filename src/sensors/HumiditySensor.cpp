#include "sensors/HumiditySensor.h"

namespace iot {

HumiditySensor::HumiditySensor(const std::string& name, int pin,
                                double minHum, double maxHum)
    : BaseSensor(name, SensorType::Humidity, pin)
    , minHum_(minHum)
    , maxHum_(maxHum)
{}

bool HumiditySensor::onInitialize() {
    Logger::getInstance().debug(
        "Humidity sensor range: [" + std::to_string(minHum_) +
        ", " + std::to_string(maxHum_) + "] %RH", getName());
    return true;
}

double HumiditySensor::readRawValue() {
    double drift = generateRandomValue(-1.0, 1.0);
    lastReading_ += drift;
    if (lastReading_ < minHum_) lastReading_ = minHum_ + 2.0;
    if (lastReading_ > maxHum_) lastReading_ = maxHum_ - 2.0;
    return lastReading_;
}

double HumiditySensor::applyCalibration(double raw) const {
    return raw + calibrationOffset_;
}

bool HumiditySensor::validateReading(double value) const {
    return value >= minHum_ && value <= maxHum_;
}

std::string HumiditySensor::getUnit() const {
    return "%RH";
}

} // namespace iot
