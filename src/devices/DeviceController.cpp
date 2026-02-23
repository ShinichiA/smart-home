#include "devices/DeviceController.h"
#include "core/Logger.h"
#include <stdexcept>

namespace iot {

void DeviceController::registerDevice(const std::string& deviceId) {
    if (devices_.find(deviceId) != devices_.end()) {
        Logger::getInstance().warning(
            "Device already registered: " + deviceId, "DeviceController");
        return;
    }
    devices_[deviceId] = std::make_unique<DeviceContext>(deviceId);
    Logger::getInstance().info(
        "Device registered: " + deviceId, "DeviceController");
}

void DeviceController::removeDevice(const std::string& deviceId) {
    devices_.erase(deviceId);
    Logger::getInstance().info(
        "Device removed: " + deviceId, "DeviceController");
}

DeviceContext& DeviceController::getContext(const std::string& deviceId) {
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) {
        throw std::runtime_error("Device not found: " + deviceId);
    }
    return *it->second;
}

void DeviceController::activateDevice(const std::string& deviceId) {
    auto& ctx = getContext(deviceId);
    auto cmd = std::make_unique<LambdaCommand>(
        "Activate " + deviceId,
        [&ctx]() { ctx.handleEvent("activate"); },
        [&ctx]() { ctx.handleEvent("deactivate"); }
    );
    invoker_.executeCommand(std::move(cmd));
}

void DeviceController::deactivateDevice(const std::string& deviceId) {
    auto& ctx = getContext(deviceId);
    auto cmd = std::make_unique<LambdaCommand>(
        "Deactivate " + deviceId,
        [&ctx]() { ctx.handleEvent("deactivate"); },
        [&ctx]() { ctx.handleEvent("activate"); }
    );
    invoker_.executeCommand(std::move(cmd));
}

void DeviceController::triggerError(const std::string& deviceId) {
    auto& ctx = getContext(deviceId);
    auto cmd = std::make_unique<LambdaCommand>(
        "Error on " + deviceId,
        [&ctx]() { ctx.handleEvent("error"); },
        [&ctx]() { ctx.handleEvent("reset"); }
    );
    invoker_.executeCommand(std::move(cmd));
}

void DeviceController::resetDevice(const std::string& deviceId) {
    auto& ctx = getContext(deviceId);
    auto cmd = std::make_unique<LambdaCommand>(
        "Reset " + deviceId,
        [&ctx]() { ctx.handleEvent("reset"); },
        [&ctx]() { ctx.handleEvent("error"); }
    );
    invoker_.executeCommand(std::move(cmd));
}

void DeviceController::startMaintenance(const std::string& deviceId) {
    auto& ctx = getContext(deviceId);
    auto cmd = std::make_unique<LambdaCommand>(
        "Maintenance start " + deviceId,
        [&ctx]() { ctx.handleEvent("maintenance"); },
        [&ctx]() { ctx.handleEvent("done"); }
    );
    invoker_.executeCommand(std::move(cmd));
}

void DeviceController::completeMaintenance(const std::string& deviceId) {
    auto& ctx = getContext(deviceId);
    auto cmd = std::make_unique<LambdaCommand>(
        "Maintenance complete " + deviceId,
        [&ctx]() { ctx.handleEvent("done"); },
        [&ctx]() { ctx.handleEvent("maintenance"); }
    );
    invoker_.executeCommand(std::move(cmd));
}

void DeviceController::undoLastCommand() {
    invoker_.undoLast();
}

void DeviceController::redoLastCommand() {
    invoker_.redoLast();
}

std::vector<std::string> DeviceController::getCommandHistory() const {
    return invoker_.getHistory();
}

DeviceStateType DeviceController::getDeviceState(
    const std::string& deviceId) const {
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) {
        return DeviceStateType::Idle;
    }
    return it->second->getCurrentStateType();
}

std::vector<std::string> DeviceController::getRegisteredDevices() const {
    std::vector<std::string> ids;
    ids.reserve(devices_.size());
    for (const auto& [id, _] : devices_) {
        ids.push_back(id);
    }
    return ids;
}

size_t DeviceController::deviceCount() const {
    return devices_.size();
}

} // namespace iot
