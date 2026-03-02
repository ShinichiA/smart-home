#pragma once

#include <string>
#include <cstdint>

namespace iot {

/**
 * @brief Device state type enumeration.
 */
enum class DeviceStateType : uint8_t {
    Idle        = 0,
    Active      = 1,
    Error       = 2,
    Maintenance = 3
};

/**
 * @brief Convert DeviceStateType enum to string.
 */
[[nodiscard]] inline std::string stateToString(DeviceStateType state) {
    switch (state) {
        case DeviceStateType::Idle:        return "Idle";
        case DeviceStateType::Active:      return "Active";
        case DeviceStateType::Error:       return "Error";
        case DeviceStateType::Maintenance: return "Maintenance";
    }
    return "Unknown";
}

} // namespace iot
