#include "app/Application.h"

namespace iot {

Application::Application()
    : sensorService_(std::make_unique<SensorService>())
    , commService_(std::make_unique<CommunicationService>())
    , deviceController_(std::make_unique<DeviceController>())
    , automationService_(std::make_unique<AutomationService>(
          *deviceController_))
{}

Application::~Application() {
    if (initialized_.load()) {
        shutdown();
    }
}

bool Application::initialize(const std::string& configPath) {
    auto& log = Logger::getInstance();

    printBanner();

    // ── 1. Load configuration ────────────────────────────────────────
    log.info("━━━ Phase 1: Loading Configuration ━━━", "Application");
    auto& config = ConfigManager::getInstance();
    config.loadFromFile(configPath)
          .set("runtime.start_time", std::string("2026-02-25T16:29:00Z"))
          .set("runtime.debug", true);

    // Set log level from config
    auto logLevel = config.getOrDefault<std::string>(
        "system.log_level", "DEBUG");
    if (logLevel == "INFO")         log.setLogLevel(LogLevel::INFO);
    else if (logLevel == "WARNING") log.setLogLevel(LogLevel::WARNING);
    else if (logLevel == "ERROR")   log.setLogLevel(LogLevel::ERROR);
    else                            log.setLogLevel(LogLevel::DEBUG);

    // ── 2. Setup event subscriptions ─────────────────────────────────
    log.info("━━━ Phase 2: Setting up Event Bus ━━━", "Application");
    setupEventSubscriptions();

    // ── 3. Initialize sensors ────────────────────────────────────────
    log.info("━━━ Phase 3: Initializing Sensors ━━━", "Application");
    if (!sensorService_->initialize()) {
        log.error("SensorService init failed", "Application");
        return false;
    }

    // ── 4. Initialize communication ──────────────────────────────────
    log.info("━━━ Phase 4: Setting up Communication ━━━", "Application");
    if (!commService_->initialize()) {
        log.warning("CommunicationService init failed — "
                    "running in offline mode", "Application");
    }

    // ── 5. Register devices ──────────────────────────────────────────
    log.info("━━━ Phase 5: Registering Devices ━━━", "Application");
    auto fanId = config.getOrDefault<std::string>(
        "devices.fan.id", "fan_01");
    auto alarmId = config.getOrDefault<std::string>(
        "devices.alarm.id", "alarm_01");
    deviceController_->registerDevice(fanId);
    deviceController_->registerDevice(alarmId);

    log.info("Registered " +
             std::to_string(deviceController_->deviceCount()) +
             " device(s)", "Application");

    // ── 6. Initialize automation ─────────────────────────────────────
    log.info("━━━ Phase 6: Initializing Automation ━━━", "Application");
    automationService_->initialize();

    initialized_.store(true);
    log.info("✅ Application initialized successfully", "Application");
    return true;
}

void Application::run(int maxSensorCycles) {
    if (!initialized_.load()) {
        Logger::getInstance().error(
            "Cannot run — not initialized", "Application");
        return;
    }

    auto& log = Logger::getInstance();
    log.info("━━━ Starting Application ━━━", "Application");

    // Start services (order matters: listeners first, then producers)
    commService_->startListening();
    automationService_->startListening();

    // Start sensor loop (blocking until done or stop requested)
    sensorService_->start(maxSensorCycles);

    // Wait for sensor thread to finish
    while (sensorService_->isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    sensorService_->stop();

    // Demo patterns after sensor loop
    demonstratePatterns();
}

void Application::shutdown() {
    auto& log = Logger::getInstance();
    log.info("━━━ Shutting Down Application ━━━", "Application");

    // Stop services in reverse order
    automationService_->shutdown();
    sensorService_->shutdown();
    commService_->shutdown();

    // Cleanup event subscriptions
    auto& bus = EventBus::getInstance();
    bus.unsubscribe("device.state_changed", deviceEventSubId_);
    bus.unsubscribe("alert", alertSubId_);
    bus.clearAll();

    initialized_.store(false);

    log.info("╔══════════════════════════════════════════════════╗",
             "Application");
    log.info("║   System shutdown complete. Goodbye!             ║",
             "Application");
    log.info("╚══════════════════════════════════════════════════╝",
             "Application");
}

void Application::requestStop() {
    sensorService_->stop();
}

DeviceController& Application::getDeviceController() {
    return *deviceController_;
}

// ── Private ──────────────────────────────────────────────────────────

void Application::setupEventSubscriptions() {
    auto& bus = EventBus::getInstance();
    auto& log = Logger::getInstance();

    // Log device state changes
    deviceEventSubId_ = bus.subscribe<DeviceEvent>(
        "device.state_changed",
        [&log](const DeviceEvent& event) {
            log.info("🔧 [EVENT] Device " + event.deviceId + ": " +
                     event.previousState + " → " + event.newState,
                     "EventHandler");
        });

    // Log alerts
    alertSubId_ = bus.subscribe<AlertEvent>(
        "alert",
        [&log](const AlertEvent& event) {
            std::string severity =
                (event.severity == 3) ? "🚨 HIGH" :
                (event.severity == 2) ? "⚠️  MED" : "ℹ️  LOW";
            log.warning(severity + " Alert from " + event.source +
                        ": " + event.message, "AlertHandler");
        });

    log.info("Event subscriptions set up", "Application");
}

void Application::printBanner() {
    auto& log = Logger::getInstance();
    log.info("╔══════════════════════════════════════════════════╗",
             "Application");
    log.info("║   IoT Smart Home Monitoring System v1.0.0        ║",
             "Application");
    log.info("║   C++17 — Senior Level Architecture Demo         ║",
             "Application");
    log.info("╚══════════════════════════════════════════════════╝",
             "Application");
}

void Application::demonstratePatterns() {
    auto& log = Logger::getInstance();

    // ── Command Pattern Demo ─────────────────────────────────────────
    log.info("━━━ Command Pattern Demo ━━━", "Application");

    log.info("Command history:", "Application");
    for (const auto& cmd : deviceController_->getCommandHistory()) {
        log.info("  • " + cmd, "Application");
    }

    log.info("Undoing last command...", "Application");
    deviceController_->undoLastCommand();

    log.info("Redoing last command...", "Application");
    deviceController_->redoLastCommand();

    // ── State Pattern Demo ───────────────────────────────────────────
    log.info("━━━ State Pattern Demo ━━━", "Application");

    auto fanId = ConfigManager::getInstance().getOrDefault<std::string>(
        "devices.fan.id", "fan_01");
    auto alarmId = ConfigManager::getInstance().getOrDefault<std::string>(
        "devices.alarm.id", "alarm_01");

    log.info(fanId + " state: " +
             stateToString(deviceController_->getDeviceState(fanId)),
             "Application");

    deviceController_->startMaintenance(fanId);
    deviceController_->completeMaintenance(fanId);

    deviceController_->activateDevice(alarmId);
    deviceController_->triggerError(alarmId);
    deviceController_->resetDevice(alarmId);
}

} // namespace iot
