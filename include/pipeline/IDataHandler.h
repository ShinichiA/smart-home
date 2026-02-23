#pragma once

#include "sensors/ISensor.h"
#include <memory>
#include <string>

namespace iot {

/**
 * @brief Interface for data processing handlers in the pipeline.
 *
 * Design Patterns: Chain of Responsibility
 * Each handler processes SensorReading and passes to the next.
 * Modern C++: std::shared_ptr for chain links, virtual interface
 */
class IDataHandler {
public:
    virtual ~IDataHandler() = default;

    /**
     * @brief Set the next handler in the chain.
     * @return Reference to the next handler (for fluent chaining)
     */
    virtual IDataHandler& setNext(std::shared_ptr<IDataHandler> next) = 0;

    /**
     * @brief Process a sensor reading.
     * @return Processed reading (may be modified or invalidated)
     */
    virtual SensorReading handle(SensorReading reading) = 0;

    [[nodiscard]] virtual std::string getHandlerName() const = 0;
};

/**
 * @brief Base handler with default chain forwarding.
 */
class BaseDataHandler : public IDataHandler {
public:
    IDataHandler& setNext(std::shared_ptr<IDataHandler> next) override {
        next_ = std::move(next);
        return *next_;
    }

    SensorReading handle(SensorReading reading) override {
        if (next_) {
            return next_->handle(std::move(reading));
        }
        return reading;
    }

protected:
    std::shared_ptr<IDataHandler> next_;
};

} // namespace iot
