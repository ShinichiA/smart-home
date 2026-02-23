#include "communication/ProtocolAdapter.h"
#include "core/Logger.h"
#include <chrono>

namespace iot {

ProtocolAdapter::ProtocolAdapter(std::unique_ptr<IProtocol> adaptee,
                                  std::string topicPrefix)
    : adaptee_(std::move(adaptee))
    , topicPrefix_(std::move(topicPrefix))
{}

bool ProtocolAdapter::connect() {
    Logger::getInstance().info(
        "ProtocolAdapter connecting via " + adaptee_->getProtocolName(),
        "ProtocolAdapter");
    return adaptee_->connect();
}

void ProtocolAdapter::disconnect() {
    adaptee_->disconnect();
}

bool ProtocolAdapter::isConnected() const {
    return adaptee_->isConnected();
}

bool ProtocolAdapter::send(const std::string& topic,
                            const std::string& payload) {
    auto fullTopic = formatTopic(topic);
    auto wrappedPayload = wrapPayload(payload);

    Logger::getInstance().debug(
        "Adapted SEND [" + fullTopic + "]", "ProtocolAdapter");

    return adaptee_->send(fullTopic, wrappedPayload);
}

std::string ProtocolAdapter::receive(const std::string& topic) {
    return adaptee_->receive(formatTopic(topic));
}

std::string ProtocolAdapter::getProtocolName() const {
    return "Adapted(" + adaptee_->getProtocolName() + ")";
}

void ProtocolAdapter::setMessageCallback(MessageCallback callback) {
    adaptee_->setMessageCallback(std::move(callback));
}

void ProtocolAdapter::setTopicPrefix(const std::string& prefix) {
    topicPrefix_ = prefix;
}

IProtocol& ProtocolAdapter::getAdaptee() {
    return *adaptee_;
}

std::string ProtocolAdapter::formatTopic(const std::string& topic) const {
    if (topicPrefix_.empty()) return topic;
    return topicPrefix_ + "/" + topic;
}

std::string ProtocolAdapter::wrapPayload(const std::string& payload) const {
    // Add timestamp envelope
    auto now = std::chrono::system_clock::now();
    auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    return R"({"timestamp":)" + std::to_string(epoch) +
           R"(,"data":)" + payload + "}";
}

} // namespace iot
