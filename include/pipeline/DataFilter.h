#pragma once

#include "pipeline/IDataHandler.h"
#include <functional>
#include <deque>
#include <numeric>

namespace iot {

/**
 * @brief Strategy-based data filter in the pipeline.
 *
 * Design Patterns: Strategy (filter algorithm is interchangeable)
 * Modern C++: std::function for strategy, std::deque for sliding window,
 *             lambda-friendly API
 */
enum class FilterStrategy : uint8_t {
    None           = 0,
    MovingAverage  = 1,
    ExponentialMA  = 2,
    Threshold      = 3
};

class DataFilter : public BaseDataHandler {
public:
    using FilterFunc = std::function<double(double, std::deque<double>&)>;

    explicit DataFilter(FilterStrategy strategy = FilterStrategy::MovingAverage,
                        size_t windowSize = 5);

    SensorReading handle(SensorReading reading) override;
    [[nodiscard]] std::string getHandlerName() const override;

    // ── Strategy selection ───────────────────────────────────────────
    void setStrategy(FilterStrategy strategy);
    void setCustomStrategy(FilterFunc func);
    void setWindowSize(size_t size);

private:
    static double movingAverage(double newVal, std::deque<double>& window);
    static double exponentialMA(double newVal, std::deque<double>& window);
    static double thresholdFilter(double newVal, std::deque<double>& window);

    FilterFunc      currentStrategy_;
    FilterStrategy  strategyType_;
    size_t          windowSize_;
    std::deque<double> window_;
};

} // namespace iot
