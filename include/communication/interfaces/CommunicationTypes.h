#pragma once

#include <cstdint>

namespace iot {

/**
 * @brief Protocol type enumeration for communication factory.
 */
enum class ProtocolType : uint8_t {
    MQTT = 0,
    HTTP = 1
};

} // namespace iot
