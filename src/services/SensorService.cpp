#include "services/SensorService.h"

#include <algorithm>
#include <chrono>

namespace iot {

SensorService::SensorService() = default;

SensorService::~SensorService() {
    stop();
    shutdown();
}

bool SensorService::initialize() {
    auto& log = Logger::getInstance();
    log.info("Initializing SensorService...", "SensorService");

    // 1. Create sensors from config
    createSensorsFromConfig();

    // 2. Initialize all sensors
    for (auto& sensor : sensors_) {
        if (sensor && !sensor->initialize()) {
            log.error("Failed to initialize sensor: " + sensor->getName(),
                      "SensorService");
        }
    }

    // 3. Apply calibration offsets from config
    calibrateSensorsFromConfig();

    // 4. Build data pipeline
    pipeline_ = DataPipeline::createDefault();

    auto names = pipeline_.getHandlerNames();
    std::string pipelineStr;
    for (size_t i = 0; i < names.size(); ++i) {
        pipelineStr += names[i];
        if (i + 1 < names.size()) pipelineStr += " → ";
    }
    log.info("Pipeline: " + pipelineStr, "SensorService");
    log.info("SensorService initialized with " +
             std::to_string(sensors_.size()) + " sensor(s)",
             "SensorService");

    return !sensors_.empty();
}

void SensorService::start(int maxIterations) {
    if (running_.load()) {
        Logger::getInstance().warning(
            "SensorService already running", "SensorService");
        return;
    }

    running_.store(true);
    workerThread_ = std::thread(&SensorService::sensorLoop, this,
                                 maxIterations);

    Logger::getInstance().info(
        "SensorService started (maxIter=" +
        std::to_string(maxIterations) + ")", "SensorService");
}

void SensorService::stop() {
    running_.store(false);

    if (workerThread_.joinable()) {
        Logger::getInstance().info(
            "Stopping SensorService...", "SensorService");
        workerThread_.join();
    }
}

void SensorService::readOnce() {
    auto& log = Logger::getInstance();
    auto& eventBus = EventBus::getInstance();

    for (auto& sensor : sensors_) {
        if (!sensor || !sensor->isInitialized()) continue;

        // Template Method: read() = readRaw → calibrate → validate
        auto reading = sensor->read();

        // Chain of Responsibility: pipeline processing
        auto processed = pipeline_.process(reading);

        if (processed.isValid) {
            // Observer: publish processed reading
            SensorEvent evt{
                processed.sensorName,
                sensorTypeToString(processed.type),
                processed.processedValue,
                processed.timestampMs
            };
            eventBus.publish("sensor.reading", evt);
        } else {
            log.debug("Skipped invalid reading from " +
                      processed.sensorName, "SensorService");
        }
    }
}

void SensorService::shutdown() {
    Logger::getInstance().info(
        "Shutting down sensors...", "SensorService");
    for (auto& sensor : sensors_) {
        if (sensor) sensor->shutdown();
    }
}

size_t SensorService::sensorCount() const {
    return sensors_.size();
}

bool SensorService::isRunning() const {
    return running_.load();
}

// ── Private ──────────────────────────────────────────────────────────

void SensorService::sensorLoop(int maxIterations) {
    auto& log = Logger::getInstance();
    int iteration = 0;

    while (running_.load()) {
        if (maxIterations > 0 && iteration >= maxIterations) {
            break;
        }
        ++iteration;

        log.info("─── Sensor cycle " + std::to_string(iteration) +
                 (maxIterations > 0
                  ? "/" + std::to_string(maxIterations)
                  : "") + " ───",
                 "SensorService");

        readOnce();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    running_.store(false);
    log.info("Sensor loop finished after " +
             std::to_string(iteration) + " cycle(s)", "SensorService");
}

void SensorService::createSensorsFromConfig() {
    auto& cfg = ConfigManager::getInstance();
    auto& log = Logger::getInstance();

    // Read sensor types from config
    struct SensorDef {
        std::string typeStr;
        std::string configKey;
        std::string namePrefix;
    };

    // Check which sensors are enabled in config
    std::vector<SensorDef> sensorDefs = {
        {"temperature", "sensors.temperature", "DHT22_Temp"},
        {"humidity",    "sensors.humidity",     "DHT22_Hum"},
        {"motion",      "sensors.motion",       "PIR_Motion"}
    };

    for (const auto& def : sensorDefs) {
        bool enabled = cfg.getOrDefault<bool>(
            def.configKey + ".enabled", false);
        if (!enabled) {
            log.debug("Sensor " + def.typeStr + " disabled in config",
                      "SensorService");
            continue;
        }

        int pin = cfg.getOrDefault<int>(def.configKey + ".pin", 0);
        auto sensor = SensorFactory::create(def.typeStr,
                                             def.namePrefix, pin);
        if (sensor) {
            log.info("Created sensor: " + def.namePrefix +
                     " (pin=" + std::to_string(pin) + ")",
                     "SensorService");
            sensors_.push_back(std::move(sensor));
        }
    }
}

void SensorService::calibrateSensorsFromConfig() {
    auto& cfg = ConfigManager::getInstance();

    for (auto& sensor : sensors_) {
        if (!sensor) continue;

        std::string typeStr = sensorTypeToString(sensor->getType());
        std::transform(typeStr.begin(), typeStr.end(),
                       typeStr.begin(), ::tolower);

        auto offset = cfg.get<double>(
            "sensors." + typeStr + ".calibration_offset");
        if (offset.has_value()) {
            sensor->calibrate(offset.value());
        }
    }
}

} // namespace iot
