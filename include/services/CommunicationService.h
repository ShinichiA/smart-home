#pragma once

#include "communication/IProtocol.h"
#include "communication/CommunicationFactory.h"
#include "core/EventBus.h"
#include "core/Logger.h"
#include "sensors/interfaces/SensorReading.h"

#include <memory>
#include <string>

namespace iot {

/**
 * @brief Service responsible for backend communication.
 *
 * Responsibilities:
 *   - Create and manage protocol connection (MQTT/HTTP)
 *   - Subscribe to sensor events → serialize & send to backend
 *   - Handle connection lifecycle (connect/reconnect/disconnect)
 *
 * Design: Listens to EventBus for sensor data, decoupled from SensorService.
 */
class CommunicationService {
public:
    CommunicationService();
    ~CommunicationService();

    CommunicationService(const CommunicationService&) = delete;
    CommunicationService& operator=(const CommunicationService&) = delete;

    /**
     * @brief Create protocol from config and connect.
     */
    bool initialize();

    /**
     * @brief Subscribe to EventBus sensor events and auto-send to backend.
     */
    void startListening();

    /**
     * @brief Stop listening and disconnect.
     */
    void shutdown();

    /**
     * @brief Manually send a reading to backend.
     */
    bool sendReading(const SensorReading& reading);

    [[nodiscard]] bool isConnected() const;
    [[nodiscard]] std::string getProtocolName() const;

private:
    static std::string readingToJson(const SensorReading& reading);

    std::unique_ptr<IProtocol> protocol_;
    EventBus::SubscriptionId sensorSubId_{0};
    bool listening_{false};
};

} // namespace iot
