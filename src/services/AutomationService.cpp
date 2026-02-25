#include "services/AutomationService.h"
#include "sensors/interfaces/SensorTypes.h"

namespace iot {

AutomationService::AutomationService(DeviceController& deviceController)
    : deviceController_(deviceController)
{}

AutomationService::~AutomationService() {
    shutdown();
}

void AutomationService::initialize() {
    auto& log = Logger::getInstance();
    auto& cfg = ConfigManager::getInstance();

    log.info("Initializing AutomationService...", "AutomationSvc");

    // Load rules from config
    auto tempThreshold = cfg.getOrDefault<double>(
        "devices.fan.auto_trigger_temp", 30.0);
    auto motionTrigger = cfg.getOrDefault<bool>(
        "devices.alarm.motion_trigger", true);

    auto fanId = cfg.getOrDefault<std::string>(
        "devices.fan.id", "fan_01");
    auto alarmId = cfg.getOrDefault<std::string>(
        "devices.alarm.id", "alarm_01");

    // Rule 1: High temperature → activate fan
    addRule({
        "HighTemp_ActivateFan",
        SensorType::Temperature,
        tempThreshold,
        true,                   // trigger above
        fanId,
        "activate",
        2,                      // medium severity alert
        "High temperature detected"
    });

    // Rule 2: Motion detected → activate alarm
    if (motionTrigger) {
        addRule({
            "Motion_ActivateAlarm",
            SensorType::Motion,
            0.5,
            true,
            alarmId,
            "activate",
            3,                  // high severity alert
            "Motion detected — intruder alert!"
        });
    }

    log.info("AutomationService initialized with " +
             std::to_string(rules_.size()) + " rule(s)",
             "AutomationSvc");
}

void AutomationService::addRule(Rule rule) {
    Logger::getInstance().info(
        "Rule added: " + rule.name + " (" +
        sensorTypeToString(rule.sensorType) + " " +
        (rule.triggerAbove ? ">" : "<") + " " +
        std::to_string(rule.threshold) + " → " +
        rule.action + " " + rule.targetDeviceId + ")",
        "AutomationSvc");
    rules_.push_back(std::move(rule));
}

void AutomationService::startListening() {
    if (listening_) return;

    sensorSubId_ = EventBus::getInstance().subscribe<SensorEvent>(
        "sensor.reading",
        [this](const SensorEvent& event) {
            evaluateRules(event);
        });

    listening_ = true;
    Logger::getInstance().info(
        "AutomationService listening for sensor events",
        "AutomationSvc");
}

void AutomationService::shutdown() {
    if (listening_) {
        EventBus::getInstance().unsubscribe("sensor.reading",
                                             sensorSubId_);
        listening_ = false;
        Logger::getInstance().info(
            "AutomationService stopped", "AutomationSvc");
    }
}

size_t AutomationService::ruleCount() const {
    return rules_.size();
}

// ── Private ──────────────────────────────────────────────────────────

void AutomationService::evaluateRules(const SensorEvent& event) {
    // Convert event sensorType string back to enum for matching
    SensorType eventType;
    if (event.sensorType == "Temperature")    eventType = SensorType::Temperature;
    else if (event.sensorType == "Humidity")   eventType = SensorType::Humidity;
    else if (event.sensorType == "Motion")     eventType = SensorType::Motion;
    else return;

    for (const auto& rule : rules_) {
        if (rule.sensorType != eventType) continue;

        bool triggered = rule.triggerAbove
                         ? (event.value > rule.threshold)
                         : (event.value < rule.threshold);

        if (!triggered) continue;

        // Check if device needs action (avoid re-triggering)
        auto currentState = deviceController_.getDeviceState(
            rule.targetDeviceId);

        if (rule.action == "activate" &&
            currentState == DeviceStateType::Idle) {
            Logger::getInstance().warning(
                "🤖 Rule [" + rule.name + "] triggered: " +
                event.sensorType + "=" +
                std::to_string(event.value), "AutomationSvc");
            executeAction(rule);
        } else if (rule.action == "deactivate" &&
                   currentState == DeviceStateType::Active) {
            executeAction(rule);
        }
    }
}

void AutomationService::executeAction(const Rule& rule) {
    if (rule.action == "activate") {
        deviceController_.activateDevice(rule.targetDeviceId);
    } else if (rule.action == "deactivate") {
        deviceController_.deactivateDevice(rule.targetDeviceId);
    } else if (rule.action == "reset") {
        deviceController_.resetDevice(rule.targetDeviceId);
    }

    // Publish alert if configured
    if (rule.alertSeverity > 0) {
        AlertEvent alert{
            rule.name,
            rule.alertMessage + " (value triggered rule)",
            rule.alertSeverity
        };
        EventBus::getInstance().publish("alert", alert);
    }
}

} // namespace iot
