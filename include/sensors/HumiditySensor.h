#pragma once

#include "sensors/BaseSensor.h"

namespace iot {

/**
 * @brief Humidity sensor simulation (e.g., DHT22).
 */
class HumiditySensor : public BaseSensor {
public:
    HumiditySensor(const std::string& name, int pin,
                   double minHum = 0.0, double maxHum = 100.0);

protected:
    double readRawValue() override;
    double applyCalibration(double raw) const override;
    bool   validateReading(double value) const override;
    std::string getUnit() const override;
    bool   onInitialize() override;

private:
    double minHum_;
    double maxHum_;
    double lastReading_{55.0};
};

} // namespace iot
