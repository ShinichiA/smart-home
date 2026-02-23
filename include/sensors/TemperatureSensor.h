#pragma once

#include "sensors/BaseSensor.h"

namespace iot {

/**
 * @brief Temperature sensor (e.g., DHT22, DS18B20 simulation).
 *
 * Overrides Template Method hooks for temperature-specific behavior.
 */
class TemperatureSensor : public BaseSensor {
public:
    TemperatureSensor(const std::string& name, int pin,
                      double minTemp = -40.0, double maxTemp = 85.0);

protected:
    double readRawValue() override;
    double applyCalibration(double raw) const override;
    bool   validateReading(double value) const override;
    std::string getUnit() const override;
    bool   onInitialize() override;

private:
    double minTemp_;
    double maxTemp_;
    double lastReading_{22.0};  // Simulate gradual change
};

} // namespace iot
