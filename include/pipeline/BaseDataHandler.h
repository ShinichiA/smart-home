#pragma once

#include "pipeline/interfaces/IDataHandler.h"

namespace iot {

/**
 * @brief Base handler with default chain forwarding.
 *
 * Provides default implementation of setNext() and handle() that
 * forwards to the next handler in the chain.
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
