#include "pipeline/DataPipeline.h"
#include "core/ConfigManager.h"
#include "core/Logger.h"

namespace iot {

DataPipeline& DataPipeline::addHandler(
    std::shared_ptr<IDataHandler> handler) {

    handlers_.push_back(handler);

    // Rebuild chain
    head_ = handlers_.front();
    for (size_t i = 0; i + 1 < handlers_.size(); ++i) {
        handlers_[i]->setNext(handlers_[i + 1]);
    }

    Logger::getInstance().debug(
        "Pipeline handler added: " + handler->getHandlerName() +
        " (total=" + std::to_string(handlers_.size()) + ")",
        "DataPipeline");

    return *this;
}

SensorReading DataPipeline::process(SensorReading reading) {
    if (!head_) {
        Logger::getInstance().warning(
            "Empty pipeline, returning raw reading", "DataPipeline");
        return reading;
    }
    return head_->handle(std::move(reading));
}

DataPipeline DataPipeline::createDefault() {
    auto& cfg = ConfigManager::getInstance();

    DataPipeline pipeline;

    // Step 1: Validator
    double minValid = cfg.getOrDefault<double>("pipeline.threshold_min", 0.5);
    double maxValid = cfg.getOrDefault<double>("pipeline.threshold_max", 100.0);
    pipeline.addHandler(std::make_shared<DataValidator>(minValid, maxValid));

    // Step 2: Filter (Strategy based on config)
    auto filterStr = cfg.getOrDefault<std::string>(
        "pipeline.filter_strategy", "moving_average");
    auto windowSize = static_cast<size_t>(
        cfg.getOrDefault<int>("pipeline.moving_average_window", 5));

    FilterStrategy strategy = FilterStrategy::MovingAverage;
    if (filterStr == "exponential")    strategy = FilterStrategy::ExponentialMA;
    else if (filterStr == "threshold") strategy = FilterStrategy::Threshold;
    else if (filterStr == "none")      strategy = FilterStrategy::None;

    pipeline.addHandler(
        std::make_shared<DataFilter>(strategy, windowSize));

    // Step 3: Transformer (e.g., °C → °F conversion available)
    auto transformer = std::make_shared<DataTransformer>();
    // Example: Scale humidity for display
    transformer->addTransform(SensorType::Humidity,
        [](double val) -> double {
            return val;  // Identity — can be customized
        });
    pipeline.addHandler(transformer);

    Logger::getInstance().info(
        "Default pipeline created with " +
        std::to_string(pipeline.handlers_.size()) + " handlers",
        "DataPipeline");

    return pipeline;
}

std::vector<std::string> DataPipeline::getHandlerNames() const {
    std::vector<std::string> names;
    names.reserve(handlers_.size());
    for (const auto& h : handlers_) {
        names.push_back(h->getHandlerName());
    }
    return names;
}

} // namespace iot
