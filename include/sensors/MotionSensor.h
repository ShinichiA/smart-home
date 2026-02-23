#pragma once

#include "sensors/BaseSensor.h"

namespace iot {

/**
 * @brief PIR Motion sensor simulation.
 * Returns 0.0 (no motion) or 1.0 (motion detected) with configurable sensitivity.
 */
class MotionSensor : public BaseSensor {
public:
    MotionSensor(const std::string& name, int pin,
                 double sensitivity = 0.8);

protected:
    double readRawValue() override;
    bool   validateReading(double value) const override;
    std::string getUnit() const override;
    bool   onInitialize() override;

private:
    double sensitivity_;  // Probability threshold for motion detection
};

} // namespace iot
