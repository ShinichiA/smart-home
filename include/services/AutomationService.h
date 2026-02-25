#pragma once

#include "devices/DeviceController.h"
#include "sensors/interfaces/SensorTypes.h"
#include "core/EventBus.h"
#include "core/ConfigManager.h"
#include "core/Logger.h"

#include <vector>
#include <functional>
#include <string>

namespace iot {

/**
 * @brief Service responsible for automation rules & device triggering.
 *
 * Responsibilities:
 *   - Define automation rules (sensor thresholds → device actions)
 *   - Subscribe to sensor events and evaluate rules
 *   - Dispatch device commands via DeviceController
 *   - Publish alert events
 *
 * Design: Decoupled from sensors — only listens to EventBus.
 *         Rules can be extended/modified at runtime.
 */
class AutomationService {
public:
    /**
     * @brief A single automation rule.
     */
    struct Rule {
        std::string     name;
        SensorType      sensorType;
        double          threshold;
        bool            triggerAbove;    // true = trigger when above threshold
        std::string     targetDeviceId;
        std::string     action;         // "activate", "deactivate", etc.
        int             alertSeverity;  // 0 = no alert, 1-3 = severity
        std::string     alertMessage;
    };

    explicit AutomationService(DeviceController& deviceController);
    ~AutomationService();

    AutomationService(const AutomationService&) = delete;
    AutomationService& operator=(const AutomationService&) = delete;

    /**
     * @brief Load default rules from config.
     */
    void initialize();

    /**
     * @brief Add a custom automation rule.
     */
    void addRule(Rule rule);

    /**
     * @brief Start listening to sensor events and evaluating rules.
     */
    void startListening();

    /**
     * @brief Stop listening.
     */
    void shutdown();

    [[nodiscard]] size_t ruleCount() const;

private:
    void evaluateRules(const SensorEvent& event);
    void executeAction(const Rule& rule);

    DeviceController& deviceController_;
    std::vector<Rule> rules_;
    EventBus::SubscriptionId sensorSubId_{0};
    bool listening_{false};
};

} // namespace iot
