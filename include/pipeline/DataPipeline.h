#pragma once

#include "pipeline/BaseDataHandler.h"
#include "pipeline/DataValidator.h"
#include "pipeline/DataFilter.h"
#include "pipeline/DataTransformer.h"
#include <memory>
#include <vector>

namespace iot {

/**
 * @brief Orchestrates the data processing pipeline.
 *
 * Builds a Chain of Responsibility: Validator → Filter → Transformer
 * Uses Builder-like fluent API for pipeline construction.
 *
 * Modern C++: shared_ptr chain, move semantics, fluent API
 */
class DataPipeline {
public:
    DataPipeline() = default;

    /**
     * @brief Fluent builder: add a handler to the pipeline.
     */
    DataPipeline& addHandler(std::shared_ptr<IDataHandler> handler);

    /**
     * @brief Process a reading through the entire pipeline.
     */
    SensorReading process(SensorReading reading);

    /**
     * @brief Create a default pipeline from config.
     */
    static DataPipeline createDefault();

    /**
     * @brief Get list of handler names in order.
     */
    std::vector<std::string> getHandlerNames() const;

private:
    std::vector<std::shared_ptr<IDataHandler>> handlers_;
    std::shared_ptr<IDataHandler> head_;
};

} // namespace iot
