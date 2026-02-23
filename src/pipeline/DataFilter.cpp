#include "pipeline/DataFilter.h"
#include "core/Logger.h"
#include <cmath>

namespace iot {

DataFilter::DataFilter(FilterStrategy strategy, size_t windowSize)
    : strategyType_(strategy)
    , windowSize_(windowSize)
{
    setStrategy(strategy);
}

void DataFilter::setStrategy(FilterStrategy strategy) {
    strategyType_ = strategy;
    switch (strategy) {
        case FilterStrategy::None:
            currentStrategy_ = [](double val, std::deque<double>&) {
                return val;
            };
            break;
        case FilterStrategy::MovingAverage:
            currentStrategy_ = movingAverage;
            break;
        case FilterStrategy::ExponentialMA:
            currentStrategy_ = exponentialMA;
            break;
        case FilterStrategy::Threshold:
            currentStrategy_ = thresholdFilter;
            break;
    }
}

void DataFilter::setCustomStrategy(FilterFunc func) {
    currentStrategy_ = std::move(func);
}

void DataFilter::setWindowSize(size_t size) {
    windowSize_ = size;
    while (window_.size() > windowSize_) {
        window_.pop_front();
    }
}

SensorReading DataFilter::handle(SensorReading reading) {
    if (!reading.isValid) {
        return BaseDataHandler::handle(std::move(reading));
    }

    double original = reading.processedValue;
    reading.processedValue = currentStrategy_(original, window_);

    // Maintain window size
    window_.push_back(original);
    if (window_.size() > windowSize_) {
        window_.pop_front();
    }

    Logger::getInstance().debug(
        "Filter [" + reading.sensorName + "]: " +
        std::to_string(original) + " -> " +
        std::to_string(reading.processedValue),
        "DataFilter");

    return BaseDataHandler::handle(std::move(reading));
}

std::string DataFilter::getHandlerName() const {
    return "DataFilter";
}

// ── Strategy implementations ─────────────────────────────────────────

double DataFilter::movingAverage(double newVal,
                                  std::deque<double>& window) {
    double sum = std::accumulate(window.begin(), window.end(), newVal);
    return sum / static_cast<double>(window.size() + 1);
}

double DataFilter::exponentialMA(double newVal,
                                  std::deque<double>& window) {
    constexpr double alpha = 0.3;
    if (window.empty()) return newVal;
    double prevEma = window.back();
    return alpha * newVal + (1.0 - alpha) * prevEma;
}

double DataFilter::thresholdFilter(double newVal,
                                    std::deque<double>& window) {
    if (window.empty()) return newVal;
    double lastVal = window.back();
    constexpr double maxDelta = 5.0;
    if (std::abs(newVal - lastVal) > maxDelta) {
        return lastVal;  // Reject spike
    }
    return newVal;
}

} // namespace iot
