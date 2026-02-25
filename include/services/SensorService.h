#pragma once

#include "sensors/interfaces/ISensor.h"
#include "sensors/SensorFactory.h"
#include "pipeline/DataPipeline.h"
#include "core/EventBus.h"
#include "core/ConfigManager.h"
#include "core/Logger.h"

#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>

namespace iot {

/**
 * @brief Service responsible for sensor lifecycle and data acquisition.
 *
 * Responsibilities:
 *   - Create and initialize sensors from config (via SensorFactory)
 *   - Run multi-threaded sensor reading loop
 *   - Process readings through DataPipeline
 *   - Publish processed readings as SensorEvent via EventBus
 *
 * Design: Single Responsibility Principle — only sensor concerns.
 */
class SensorService {
public:
    SensorService();
    ~SensorService();

    // Non-copyable, non-movable (owns thread)
    SensorService(const SensorService&) = delete;
    SensorService& operator=(const SensorService&) = delete;

    /**
     * @brief Initialize sensors from config and build data pipeline.
     */
    bool initialize();

    /**
     * @brief Start the sensor reading loop in a background thread.
     * @param maxIterations  Max number of read cycles (0 = infinite)
     */
    void start(int maxIterations = 0);

    /**
     * @brief Stop the sensor reading loop and join the thread.
     */
    void stop();

    /**
     * @brief Perform a single read cycle (for synchronous usage).
     */
    void readOnce();

    /**
     * @brief Shutdown all sensors and release resources.
     */
    void shutdown();

    [[nodiscard]] size_t sensorCount() const;
    [[nodiscard]] bool isRunning() const;

private:
    void sensorLoop(int maxIterations);
    void createSensorsFromConfig();
    void calibrateSensorsFromConfig();

    std::vector<std::unique_ptr<ISensor>> sensors_;
    DataPipeline pipeline_;
    std::thread workerThread_;
    std::atomic<bool> running_{false};
};

} // namespace iot
