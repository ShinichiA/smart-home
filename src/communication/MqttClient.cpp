#include "communication/MqttClient.h"
#include "core/Logger.h"

namespace iot {

MqttClient::MqttClient(std::string brokerHost, int port,
                        std::string clientId, int qos)
    : brokerHost_(std::move(brokerHost))
    , port_(port)
    , clientId_(std::move(clientId))
    , qos_(qos)
{}

MqttClient::~MqttClient() {
    if (connected_.load()) {
        disconnect();
    }
}

bool MqttClient::connect() {
    auto& log = Logger::getInstance();
    log.info("MQTT connecting to " + brokerHost_ + ":" +
             std::to_string(port_) + " (client=" + clientId_ + ")",
             "MqttClient");

    // Simulate connection handshake
    connected_.store(true);

    log.info("MQTT connected successfully (QoS=" +
             std::to_string(qos_) + ", KeepAlive=" +
             std::to_string(keepAliveSec_) + "s)", "MqttClient");
    return true;
}

void MqttClient::disconnect() {
    auto& log = Logger::getInstance();
    log.info("MQTT disconnecting from " + brokerHost_, "MqttClient");
    connected_.store(false);
    subscribedTopics_.clear();
}

bool MqttClient::isConnected() const {
    return connected_.load();
}

bool MqttClient::send(const std::string& topic, const std::string& payload) {
    if (!connected_.load()) {
        Logger::getInstance().error(
            "MQTT not connected, cannot publish to: " + topic, "MqttClient");
        return false;
    }

    Logger::getInstance().debug(
        "MQTT PUBLISH [" + topic + "] QoS=" + std::to_string(qos_) +
        " payload_size=" + std::to_string(payload.size()), "MqttClient");

    // Simulate: store in queue and trigger callback
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        messageQueue_.push({topic, payload});
    }

    if (messageCallback_) {
        messageCallback_(topic, payload);
    }

    return true;
}

std::string MqttClient::receive(const std::string& topic) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (messageQueue_.empty()) return "";

    auto [msgTopic, payload] = messageQueue_.front();
    if (msgTopic == topic) {
        messageQueue_.pop();
        return payload;
    }
    return "";
}

std::string MqttClient::getProtocolName() const {
    return "MQTT";
}

void MqttClient::setMessageCallback(MessageCallback callback) {
    messageCallback_ = std::move(callback);
}

void MqttClient::subscribe(const std::string& topic) {
    subscribedTopics_.push_back(topic);
    Logger::getInstance().info(
        "MQTT SUBSCRIBE [" + topic + "]", "MqttClient");
}

void MqttClient::setKeepAlive(int seconds) {
    keepAliveSec_ = seconds;
}

} // namespace iot
