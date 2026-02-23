#pragma once

#include "pipeline/IDataHandler.h"
#include <functional>
#include <map>

namespace iot {

/**
 * @brief Transforms sensor readings (unit conversion, scaling, etc.).
 * Part of Chain of Responsibility pipeline.
 *
 * Modern C++: std::function for custom transform, lambda support
 */
class DataTransformer : public BaseDataHandler {
public:
    using TransformFunc = std::function<double(double)>;

    DataTransformer() = default;

    SensorReading handle(SensorReading reading) override;
    [[nodiscard]] std::string getHandlerName() const override;

    /**
     * @brief Register a transform for a specific sensor type.
     */
    void addTransform(SensorType type, TransformFunc transform);

private:
    std::map<SensorType, TransformFunc> transforms_;
};

} // namespace iot
