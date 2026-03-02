#pragma once

#include <string>
#include <cstdint>

namespace iot {

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
