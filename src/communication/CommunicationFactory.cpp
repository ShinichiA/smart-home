#include "communication/CommunicationFactory.h"
#include "core/ConfigManager.h"
#include "core/Logger.h"

namespace iot {

std::unique_ptr<IProtocol> CommunicationFactory::createProtocol(
    ProtocolType type) {

    switch (type) {
        case ProtocolType::MQTT: {
            auto& cfg = ConfigManager::getInstance();
            auto host = cfg.getOrDefault<std::string>(
                "communication.mqtt.broker_host", "localhost");
            auto port = cfg.getOrDefault<int>(
                "communication.mqtt.broker_port", 1883);
            auto clientId = cfg.getOrDefault<std::string>(
                "communication.mqtt.client_id", "iot_default");
            auto qos = cfg.getOrDefault<int>(
                "communication.mqtt.qos", 1);

            auto client = std::make_unique<MqttClient>(
                host, port, clientId, qos);

            auto keepAlive = cfg.getOrDefault<int>(
                "communication.mqtt.keepalive_sec", 60);
            client->setKeepAlive(keepAlive);

            return client;
        }

        case ProtocolType::HTTP: {
            auto& cfg = ConfigManager::getInstance();
            auto url = cfg.getOrDefault<std::string>(
                "communication.http.base_url", "http://localhost");
            auto port = cfg.getOrDefault<int>(
                "communication.http.port", 8080);
            auto key = cfg.getOrDefault<std::string>(
                "communication.http.api_key", "");
            auto timeout = cfg.getOrDefault<int>(
                "communication.http.timeout_ms", 5000);

            auto client = std::make_unique<HttpClient>(
                url, port, key, timeout);

            auto retries = cfg.getOrDefault<int>(
                "communication.http.retry_count", 3);
            client->setRetryCount(retries);

            return client;
        }
    }

    Logger::getInstance().error("Unknown protocol type", "CommFactory");
    return nullptr;
}

std::unique_ptr<IProtocol> CommunicationFactory::createAdaptedProtocol(
    ProtocolType type, const std::string& topicPrefix) {

    auto raw = createProtocol(type);
    if (!raw) return nullptr;

    return std::make_unique<ProtocolAdapter>(std::move(raw), topicPrefix);
}

std::unique_ptr<IProtocol> CommunicationFactory::createFromConfig() {
    auto& cfg = ConfigManager::getInstance();
    auto proto = cfg.getOrDefault<std::string>(
        "communication.protocol", "mqtt");

    auto topicPrefix = cfg.getOrDefault<std::string>(
        "communication.mqtt.topic_prefix", "home/sensors");

    ProtocolType type = (proto == "http") ? ProtocolType::HTTP
                                          : ProtocolType::MQTT;

    Logger::getInstance().info(
        "Creating protocol from config: " + proto, "CommFactory");

    return createAdaptedProtocol(type, topicPrefix);
}

} // namespace iot
