#pragma once

#include "pipeline/IDataHandler.h"

namespace iot {

/**
 * @brief Validates sensor readings (range, null, staleness).
 * Part of Chain of Responsibility pipeline.
 */
class DataValidator : public BaseDataHandler {
public:
    DataValidator(double minValid, double maxValid);

    SensorReading handle(SensorReading reading) override;
    [[nodiscard]] std::string getHandlerName() const override;

private:
    double minValid_;
    double maxValid_;
};

} // namespace iot
