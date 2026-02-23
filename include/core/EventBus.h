#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <algorithm>
#include <typeindex>
#include <any>
#include <memory>

namespace iot {

/**
 * @brief Type-safe, thread-safe Event Bus implementing Observer pattern.
 *
 * Design Patterns: Observer (publish/subscribe)
 * Modern C++: std::any, std::function, std::type_index, templates,
 *             perfect forwarding, RAII, lambda support
 */
class EventBus {
public:
    // ── Subscription handle for unsubscribing ────────────────────────
    using SubscriptionId = size_t;

    static EventBus& getInstance();

    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    /**
     * @brief Subscribe to events of type T.
     * @tparam T Event data type
     * @param eventName Name/topic of the event
     * @param handler  Callback receiving const T&
     * @return SubscriptionId for later unsubscribing
     */
    template<typename T>
    SubscriptionId subscribe(const std::string& eventName,
                             std::function<void(const T&)> handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto id = nextId_++;

        auto wrapper = [handler = std::move(handler)](const std::any& data) {
            handler(std::any_cast<const T&>(data));
        };

        subscribers_[eventName].push_back({id, std::move(wrapper)});
        return id;
    }

    /**
     * @brief Publish an event to all subscribers.
     * @tparam T Event data type (deduced)
     * @param eventName Name/topic of the event
     * @param data The event payload
     */
    template<typename T>
    void publish(const std::string& eventName, const T& data) {
        std::vector<Subscriber> subs;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = subscribers_.find(eventName);
            if (it == subscribers_.end()) return;
            subs = it->second;  // copy so we don't hold lock during callback
        }
        for (const auto& sub : subs) {
            sub.callback(data);
        }
    }

    /**
     * @brief Unsubscribe by SubscriptionId.
     */
    void unsubscribe(const std::string& eventName, SubscriptionId id);

    /**
     * @brief Remove all subscribers for an event.
     */
    void clearEvent(const std::string& eventName);

    /**
     * @brief Remove all subscribers from all events.
     */
    void clearAll();

    /**
     * @brief Get subscriber count for an event.
     */
    size_t subscriberCount(const std::string& eventName) const;

private:
    EventBus() = default;
    ~EventBus() = default;

    struct Subscriber {
        SubscriptionId id;
        std::function<void(const std::any&)> callback;
    };

    std::unordered_map<std::string, std::vector<Subscriber>> subscribers_;
    mutable std::mutex mutex_;
    SubscriptionId nextId_{0};
};

// ─── Common Event Types ──────────────────────────────────────────────

struct SensorEvent {
    std::string sensorName;
    std::string sensorType;
    double      value;
    uint64_t    timestampMs;
};

struct DeviceEvent {
    std::string deviceId;
    std::string action;
    std::string previousState;
    std::string newState;
};

struct AlertEvent {
    std::string source;
    std::string message;
    int         severity;  // 1=low, 2=medium, 3=high
};

} // namespace iot
