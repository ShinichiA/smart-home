/**
 * @file main.cpp
 * @brief IoT Smart Home Monitoring System — Entry Point
 *
 * Demonstrates all design patterns and modern C++ features:
 *
 *  1. Singleton       — Logger, ConfigManager, EventBus
 *  2. Factory Method  — SensorFactory
 *  3. Abstract Factory— CommunicationFactory
 *  4. Observer        — EventBus pub/sub
 *  5. Strategy        — DataFilter algorithms
 *  6. Command         — DeviceCommand with undo/redo
 *  7. State           — DeviceState machine
 *  8. Chain of Resp.  — DataPipeline
 *  9. Builder         — ConfigManager fluent API
 * 10. Adapter         — ProtocolAdapter
 * 11. Template Method — BaseSensor::read()
 *
 * Modern C++17: smart pointers, std::variant, std::optional, std::any,
 *   std::function, lambda, threading, enum class, structured bindings,
 *   RAII, move semantics, [[nodiscard]], constexpr, auto
 */

#include "core/Logger.h"
#include "core/ConfigManager.h"
#include "core/EventBus.h"
#include "sensors/SensorFactory.h"
#include "communication/CommunicationFactory.h"
#include "pipeline/DataPipeline.h"
#include "devices/DeviceController.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <memory>
#include <atomic>
#include <csignal>

// ─── Global shutdown flag ────────────────────────────────────────────
static std::atomic<bool> g_running{true};

void signalHandler(int /*signum*/) {
    g_running.store(false);
}

// ─── Helper: format SensorReading to JSON string ─────────────────────
static std::string readingToJson(const iot::SensorReading& r) {
    std::string json = "{";
    json += R"("sensor":")" + r.sensorName + "\",";
    json += R"("type":")" + iot::sensorTypeToString(r.type) + "\",";
    json += R"("raw":)" + std::to_string(r.rawValue) + ",";
    json += R"("value":)" + std::to_string(r.processedValue) + ",";
    json += R"("valid":)" + std::string(r.isValid ? "true" : "false") + ",";
    json += R"("timestamp":)" + std::to_string(r.timestampMs);
    if (r.unit.has_value()) {
        json += R"(,"unit":")" + r.unit.value() + "\"";
    }
    json += "}";
    return json;
}

int main() {
    // ── Signal handling for graceful shutdown ─────────────────────────
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    auto& logger = iot::Logger::getInstance();
    logger.setLogLevel(iot::LogLevel::DEBUG);

    logger.info("╔══════════════════════════════════════════════════╗", "Main");
    logger.info("║   IoT Smart Home Monitoring System v1.0.0       ║", "Main");
    logger.info("║   C++17 — Senior Level Architecture Demo        ║", "Main");
    logger.info("╚══════════════════════════════════════════════════╝", "Main");

    // ══════════════════════════════════════════════════════════════════
    // 1. CONFIGURATION (Singleton + Builder)
    // ══════════════════════════════════════════════════════════════════
    logger.info("━━━ Phase 1: Loading Configuration ━━━", "Main");

    auto& config = iot::ConfigManager::getInstance();
    config.loadFromFile("config/config.json")
          .set("runtime.start_time", std::string("2026-02-23T15:55:00Z"))
          .set("runtime.debug", true);

    config.printAll();

    // ══════════════════════════════════════════════════════════════════
    // 2. EVENT BUS SETUP (Observer Pattern)
    // ══════════════════════════════════════════════════════════════════
    logger.info("━━━ Phase 2: Setting up Event Bus ━━━", "Main");

    auto& eventBus = iot::EventBus::getInstance();

    // Subscribe to sensor events
    auto sensorSubId = eventBus.subscribe<iot::SensorEvent>(
        "sensor.reading",
        [&logger](const iot::SensorEvent& event) {
            logger.info(
                "📡 [EVENT] Sensor: " + event.sensorName +
                " (" + event.sensorType + ") = " +
                std::to_string(event.value), "EventHandler");
        });

    // Subscribe to device state changes
    eventBus.subscribe<iot::DeviceEvent>(
        "device.state_changed",
        [&logger](const iot::DeviceEvent& event) {
            logger.info(
                "🔧 [EVENT] Device " + event.deviceId + ": " +
                event.previousState + " → " + event.newState,
                "EventHandler");
        });

    // Subscribe to alerts
    eventBus.subscribe<iot::AlertEvent>(
        "alert",
        [&logger](const iot::AlertEvent& event) {
            std::string severity = (event.severity == 3) ? "🚨 HIGH" :
                                   (event.severity == 2) ? "⚠️  MED" : "ℹ️  LOW";
            logger.warning(
                severity + " Alert from " + event.source +
                ": " + event.message, "AlertHandler");
        });

    logger.info("Subscribed " +
                std::to_string(eventBus.subscriberCount("sensor.reading")) +
                " handler(s) to sensor.reading", "Main");

    // ══════════════════════════════════════════════════════════════════
    // 3. CREATE SENSORS (Factory Method Pattern)
    // ══════════════════════════════════════════════════════════════════
    logger.info("━━━ Phase 3: Creating Sensors ━━━", "Main");

    std::vector<std::unique_ptr<iot::ISensor>> sensors;

    // Factory Method: create by enum type
    sensors.push_back(
        iot::SensorFactory::create(iot::SensorType::Temperature,
                                    "DHT22_Temp", 4));
    sensors.push_back(
        iot::SensorFactory::create(iot::SensorType::Humidity,
                                    "DHT22_Hum", 5));

    // Factory Method: create by string type
    sensors.push_back(
        iot::SensorFactory::create("motion", "PIR_Motion", 17));

    // Initialize all sensors
    for (auto& sensor : sensors) {
        if (sensor) {
            sensor->initialize();
            // Apply calibration from config
            auto key = "sensors." +
                       iot::sensorTypeToString(sensor->getType()) +
                       ".calibration_offset";
            // Convert type name to lowercase for config key
            std::string typeStr = iot::sensorTypeToString(sensor->getType());
            std::transform(typeStr.begin(), typeStr.end(),
                          typeStr.begin(), ::tolower);
            auto offset = config.get<double>(
                "sensors." + typeStr + ".calibration_offset");
            if (offset.has_value()) {
                sensor->calibrate(offset.value());
            }
        }
    }

    // ══════════════════════════════════════════════════════════════════
    // 4. CREATE COMMUNICATION (Abstract Factory + Adapter)
    // ══════════════════════════════════════════════════════════════════
    logger.info("━━━ Phase 4: Setting up Communication ━━━", "Main");

    // Abstract Factory: create from config
    auto protocol = iot::CommunicationFactory::createFromConfig();
    if (protocol) {
        protocol->connect();

        // Set message callback
        protocol->setMessageCallback(
            [&logger](const std::string& topic,
                      const std::string& payload) {
                logger.debug(
                    "📨 Backend ACK [" + topic + "] payload_len=" +
                    std::to_string(payload.size()), "CommCallback");
            });
    }

    // ══════════════════════════════════════════════════════════════════
    // 5. CREATE DATA PIPELINE (Chain of Responsibility + Strategy)
    // ══════════════════════════════════════════════════════════════════
    logger.info("━━━ Phase 5: Building Data Pipeline ━━━", "Main");

    auto pipeline = iot::DataPipeline::createDefault();

    auto handlerNames = pipeline.getHandlerNames();
    std::string pipelineStr;
    for (size_t i = 0; i < handlerNames.size(); ++i) {
        pipelineStr += handlerNames[i];
        if (i + 1 < handlerNames.size()) pipelineStr += " → ";
    }
    logger.info("Pipeline: " + pipelineStr, "Main");

    // ══════════════════════════════════════════════════════════════════
    // 6. DEVICE CONTROLLER (State + Command Patterns)
    // ══════════════════════════════════════════════════════════════════
    logger.info("━━━ Phase 6: Registering Devices ━━━", "Main");

    iot::DeviceController deviceController;
    deviceController.registerDevice("fan_01");
    deviceController.registerDevice("alarm_01");

    logger.info("Registered " +
                std::to_string(deviceController.deviceCount()) +
                " device(s)", "Main");

    // ══════════════════════════════════════════════════════════════════
    // 7. MAIN LOOP — Multi-threaded sensor reading
    // ══════════════════════════════════════════════════════════════════
    logger.info("━━━ Phase 7: Starting Sensor Loop ━━━", "Main");

    constexpr int MAX_ITERATIONS = 5;
    int iteration = 0;

    // Sensor reading thread
    std::thread sensorThread([&]() {
        while (g_running.load() && iteration < MAX_ITERATIONS) {
            ++iteration;
            logger.info("─── Iteration " + std::to_string(iteration) +
                        "/" + std::to_string(MAX_ITERATIONS) + " ───",
                        "SensorLoop");

            for (auto& sensor : sensors) {
                if (!sensor || !sensor->isInitialized()) continue;

                // Template Method: read() = readRaw → calibrate → validate
                auto reading = sensor->read();

                // Chain of Responsibility: pipeline processing
                auto processed = pipeline.process(reading);

                if (processed.isValid) {
                    // Observer: publish event
                    iot::SensorEvent evt{
                        processed.sensorName,
                        iot::sensorTypeToString(processed.type),
                        processed.processedValue,
                        processed.timestampMs
                    };
                    eventBus.publish("sensor.reading", evt);

                    // Adapter: send to backend
                    if (protocol && protocol->isConnected()) {
                        auto json = readingToJson(processed);
                        protocol->send(processed.sensorName, json);
                    }

                    // Smart automation: trigger devices based on readings
                    if (processed.type == iot::SensorType::Temperature &&
                        processed.processedValue > 28.0) {
                        if (deviceController.getDeviceState("fan_01") ==
                            iot::DeviceStateType::Idle) {
                            logger.warning(
                                "🌡️ Temp > 28°C — Activating fan!",
                                "Automation");
                            deviceController.activateDevice("fan_01");
                        }

                        iot::AlertEvent alert{
                            "TemperatureSensor",
                            "High temperature: " +
                            std::to_string(processed.processedValue) + "°C",
                            2
                        };
                        eventBus.publish("alert", alert);
                    }

                    if (processed.type == iot::SensorType::Motion &&
                        processed.processedValue > 0.5) {
                        if (deviceController.getDeviceState("alarm_01") ==
                            iot::DeviceStateType::Idle) {
                            logger.warning(
                                "🚶 Motion detected — Activating alarm!",
                                "Automation");
                            deviceController.activateDevice("alarm_01");
                        }
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    sensorThread.join();

    // ══════════════════════════════════════════════════════════════════
    // 8. DEMONSTRATE COMMAND UNDO/REDO
    // ══════════════════════════════════════════════════════════════════
    logger.info("━━━ Phase 8: Command Pattern Demo ━━━", "Main");

    logger.info("Command history:", "Main");
    for (const auto& cmd : deviceController.getCommandHistory()) {
        logger.info("  • " + cmd, "Main");
    }

    logger.info("Undoing last command...", "Main");
    deviceController.undoLastCommand();

    logger.info("Redoing last command...", "Main");
    deviceController.redoLastCommand();

    // ══════════════════════════════════════════════════════════════════
    // 9. DEMONSTRATE STATE TRANSITIONS
    // ══════════════════════════════════════════════════════════════════
    logger.info("━━━ Phase 9: State Pattern Demo ━━━", "Main");

    logger.info("fan_01 state: " + iot::stateToString(
        deviceController.getDeviceState("fan_01")), "Main");

    deviceController.startMaintenance("fan_01");
    deviceController.completeMaintenance("fan_01");

    deviceController.activateDevice("alarm_01");
    deviceController.triggerError("alarm_01");
    deviceController.resetDevice("alarm_01");

    // ══════════════════════════════════════════════════════════════════
    // 10. CLEANUP
    // ══════════════════════════════════════════════════════════════════
    logger.info("━━━ Phase 10: Shutting Down ━━━", "Main");

    // Shutdown sensors (RAII also handles this, but explicit is good)
    for (auto& sensor : sensors) {
        if (sensor) sensor->shutdown();
    }

    // Disconnect communication
    if (protocol) protocol->disconnect();

    // Unsubscribe (demonstrating Observer cleanup)
    eventBus.unsubscribe("sensor.reading", sensorSubId);
    eventBus.clearAll();

    logger.info("╔══════════════════════════════════════════════════╗", "Main");
    logger.info("║   System shutdown complete. Goodbye!             ║", "Main");
    logger.info("╚══════════════════════════════════════════════════╝", "Main");

    return 0;
}
