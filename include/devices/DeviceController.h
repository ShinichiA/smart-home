#pragma once

#include "devices/DeviceState.h"
#include "devices/DeviceCommand.h"
#include <unordered_map>
#include <memory>

namespace iot {

/**
 * @brief High-level device controller managing multiple devices.
 *
 * Integrates State pattern (DeviceContext) with Command pattern
 * (CommandInvoker) for a complete device management system.
 *
 * Modern C++: unordered_map of unique_ptr, structured bindings,
 *             range-based operations
 */
class DeviceController {
public:
    DeviceController() = default;

    // ── Device management ────────────────────────────────────────────
    void registerDevice(const std::string& deviceId);
    void removeDevice(const std::string& deviceId);

    // ── State control via Command pattern ────────────────────────────
    void activateDevice(const std::string& deviceId);
    void deactivateDevice(const std::string& deviceId);
    void triggerError(const std::string& deviceId);
    void resetDevice(const std::string& deviceId);
    void startMaintenance(const std::string& deviceId);
    void completeMaintenance(const std::string& deviceId);

    // ── Command history ──────────────────────────────────────────────
    void undoLastCommand();
    void redoLastCommand();
    std::vector<std::string> getCommandHistory() const;

    // ── Query ────────────────────────────────────────────────────────
    DeviceStateType getDeviceState(const std::string& deviceId) const;
    std::vector<std::string> getRegisteredDevices() const;
    size_t deviceCount() const;

private:
    DeviceContext& getContext(const std::string& deviceId);

    std::unordered_map<std::string, std::unique_ptr<DeviceContext>> devices_;
    CommandInvoker invoker_;
};

} // namespace iot
