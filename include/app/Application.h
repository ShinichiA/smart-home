#pragma once

#include "services/SensorService.h"
#include "services/CommunicationService.h"
#include "services/AutomationService.h"
#include "devices/DeviceController.h"
#include "core/Logger.h"
#include "core/ConfigManager.h"
#include "core/EventBus.h"

#include <memory>
#include <atomic>

namespace iot {

/**
 * @brief Application orchestrator — Facade pattern.
 *
 * Design Patterns: Facade
 *   - Single entry point for the entire system lifecycle
 *   - Owns and wires all services and controllers
 *   - Simple API: initialize() → run() → shutdown()
 *
 * Modern C++: std::unique_ptr ownership, RAII, std::atomic
 */
class Application {
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /**
     * @brief Initialize all subsystems.
     * @param configPath Path to JSON config file
     * @return true if all init succeeded
     */
    bool initialize(const std::string& configPath = "config/config.json");

    /**
     * @brief Run the application (starts all services).
     * @param maxSensorCycles Max sensor reading cycles (0 = infinite)
     */
    void run(int maxSensorCycles = 0);

    /**
     * @brief Gracefully shutdown all subsystems.
     */
    void shutdown();

    /**
     * @brief Request shutdown from external signal.
     */
    void requestStop();

    // ── Accessors for demo/testing ───────────────────────────────────
    DeviceController& getDeviceController();

private:
    void setupEventSubscriptions();
    void printBanner();
    void demonstratePatterns();

    // ── Owned services ───────────────────────────────────────────────
    std::unique_ptr<SensorService>        sensorService_;
    std::unique_ptr<CommunicationService> commService_;
    std::unique_ptr<DeviceController>     deviceController_;
    std::unique_ptr<AutomationService>    automationService_;

    // ── Event subscription IDs for cleanup ───────────────────────────
    EventBus::SubscriptionId deviceEventSubId_{0};
    EventBus::SubscriptionId alertSubId_{0};

    std::atomic<bool> initialized_{false};
};

} // namespace iot
