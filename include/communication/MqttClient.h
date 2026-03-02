#pragma once

#include "communication/interfaces/IProtocol.h"
#include <atomic>
#include <queue>
#include <mutex>

namespace iot {

/**
 * @brief Simulated MQTT client implementing IProtocol.
 *
 * Simulates MQTT publish/subscribe with in-memory message queue.
 * Modern C++: std::atomic, std::queue, mutex, move semantics
 */
class MqttClient : public IProtocol {
public:
    MqttClient(std::string brokerHost, int port,
               std::string clientId, int qos = 1);
    ~MqttClient() override;

    bool connect() override;
    void disconnect() override;
    [[nodiscard]] bool isConnected() const override;

    bool send(const std::string& topic, const std::string& payload) override;
    std::string receive(const std::string& topic) override;

    [[nodiscard]] std::string getProtocolName() const override;
    void setMessageCallback(MessageCallback callback) override;

    // MQTT-specific
    void subscribe(const std::string& topic);
    void setKeepAlive(int seconds);

private:
    std::string     brokerHost_;
    int             port_;
    std::string     clientId_;
    int             qos_;
    int             keepAliveSec_{60};
    std::atomic<bool> connected_{false};

    MessageCallback messageCallback_;
    std::queue<std::pair<std::string, std::string>> messageQueue_;
    std::mutex queueMutex_;
    std::vector<std::string> subscribedTopics_;
};

} // namespace iot
