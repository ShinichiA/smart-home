#pragma once

#include "sensors/interfaces/SensorReading.h"
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

protected:
    IDataHandler() = default;
    IDataHandler(const IDataHandler&) = default;
    IDataHandler& operator=(const IDataHandler&) = default;
    IDataHandler(IDataHandler&&) = default;
    IDataHandler& operator=(IDataHandler&&) = default;
};

} // namespace iot
