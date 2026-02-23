#pragma once

#include "communication/IProtocol.h"
#include "communication/MqttClient.h"
#include "communication/HttpClient.h"
#include "communication/ProtocolAdapter.h"
#include <memory>
#include <string>

namespace iot {

/**
 * @brief Abstract Factory for creating communication protocol stacks.
 *
 * Design Patterns: Abstract Factory
 * Modern C++: std::unique_ptr, enum class, move semantics
 */
enum class ProtocolType : uint8_t {
    MQTT = 0,
    HTTP = 1
};

class CommunicationFactory {
public:
    /**
     * @brief Create a raw protocol client.
     */
    static std::unique_ptr<IProtocol> createProtocol(ProtocolType type);

    /**
     * @brief Create a protocol wrapped in an adapter with topic prefix.
     */
    static std::unique_ptr<IProtocol> createAdaptedProtocol(
        ProtocolType type, const std::string& topicPrefix);

    /**
     * @brief Create protocol from config file settings.
     */
    static std::unique_ptr<IProtocol> createFromConfig();
};

} // namespace iot
