#include "core/EventBus.h"
#include "core/Logger.h"

namespace iot {

EventBus& EventBus::getInstance() {
    static EventBus instance;
    return instance;
}

void EventBus::unsubscribe(const std::string& eventName, SubscriptionId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subscribers_.find(eventName);
    if (it == subscribers_.end()) return;

    auto& subs = it->second;
    subs.erase(
        std::remove_if(subs.begin(), subs.end(),
            [id](const Subscriber& s) { return s.id == id; }),
        subs.end()
    );

    Logger::getInstance().debug(
        "Unsubscribed id=" + std::to_string(id) + " from: " + eventName,
        "EventBus");
}

void EventBus::clearEvent(const std::string& eventName) {
    std::lock_guard<std::mutex> lock(mutex_);
    subscribers_.erase(eventName);
}

void EventBus::clearAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    subscribers_.clear();
}

size_t EventBus::subscriberCount(const std::string& eventName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subscribers_.find(eventName);
    return (it != subscribers_.end()) ? it->second.size() : 0;
}

} // namespace iot
