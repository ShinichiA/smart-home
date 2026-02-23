#include "devices/DeviceState.h"
#include "core/Logger.h"
#include "core/EventBus.h"

namespace iot {

// ── DeviceContext ────────────────────────────────────────────────────

DeviceContext::DeviceContext(const std::string& deviceId)
    : deviceId_(deviceId)
    , currentState_(std::make_shared<IdleState>())
{
    currentState_->enter(*this);
}

void DeviceContext::transitionTo(std::shared_ptr<IDeviceState> newState) {
    auto& log = Logger::getInstance();
    auto oldName = currentState_->getName();

    currentState_->exit(*this);
    currentState_ = std::move(newState);
    currentState_->enter(*this);

    log.info(deviceId_ + " state: " + oldName + " → " +
             currentState_->getName(), "DeviceContext");

    // Publish state change event
    DeviceEvent event{deviceId_, "state_change", oldName,
                      currentState_->getName()};
    EventBus::getInstance().publish("device.state_changed", event);
}

void DeviceContext::handleEvent(const std::string& event) {
    currentState_->handleEvent(*this, event);
}

DeviceStateType DeviceContext::getCurrentStateType() const {
    return currentState_->getType();
}

std::string DeviceContext::getDeviceId() const {
    return deviceId_;
}

// ── IdleState ────────────────────────────────────────────────────────

void IdleState::enter(DeviceContext& ctx) {
    Logger::getInstance().debug(
        ctx.getDeviceId() + " entered Idle", "IdleState");
}

void IdleState::exit(DeviceContext& ctx) {
    Logger::getInstance().debug(
        ctx.getDeviceId() + " leaving Idle", "IdleState");
}

void IdleState::handleEvent(DeviceContext& ctx, const std::string& event) {
    if (event == "activate") {
        ctx.transitionTo(std::make_shared<ActiveState>());
    } else if (event == "maintenance") {
        ctx.transitionTo(std::make_shared<MaintenanceState>());
    } else {
        Logger::getInstance().warning(
            "Idle: unhandled event '" + event + "'", "IdleState");
    }
}

DeviceStateType IdleState::getType() const { return DeviceStateType::Idle; }
std::string IdleState::getName() const { return "Idle"; }

// ── ActiveState ──────────────────────────────────────────────────────

void ActiveState::enter(DeviceContext& ctx) {
    Logger::getInstance().debug(
        ctx.getDeviceId() + " entered Active", "ActiveState");
}

void ActiveState::exit(DeviceContext& ctx) {
    Logger::getInstance().debug(
        ctx.getDeviceId() + " leaving Active", "ActiveState");
}

void ActiveState::handleEvent(DeviceContext& ctx, const std::string& event) {
    if (event == "deactivate") {
        ctx.transitionTo(std::make_shared<IdleState>());
    } else if (event == "error") {
        ctx.transitionTo(std::make_shared<ErrorState>());
    } else if (event == "maintenance") {
        ctx.transitionTo(std::make_shared<MaintenanceState>());
    } else {
        Logger::getInstance().warning(
            "Active: unhandled event '" + event + "'", "ActiveState");
    }
}

DeviceStateType ActiveState::getType() const { return DeviceStateType::Active; }
std::string ActiveState::getName() const { return "Active"; }

// ── ErrorState ───────────────────────────────────────────────────────

void ErrorState::enter(DeviceContext& ctx) {
    Logger::getInstance().error(
        ctx.getDeviceId() + " entered ERROR state!", "ErrorState");
}

void ErrorState::exit(DeviceContext& ctx) {
    Logger::getInstance().info(
        ctx.getDeviceId() + " recovering from error", "ErrorState");
}

void ErrorState::handleEvent(DeviceContext& ctx, const std::string& event) {
    if (event == "reset") {
        ctx.transitionTo(std::make_shared<IdleState>());
    } else if (event == "maintenance") {
        ctx.transitionTo(std::make_shared<MaintenanceState>());
    } else {
        Logger::getInstance().warning(
            "Error state: only 'reset' or 'maintenance' accepted",
            "ErrorState");
    }
}

DeviceStateType ErrorState::getType() const { return DeviceStateType::Error; }
std::string ErrorState::getName() const { return "Error"; }

// ── MaintenanceState ─────────────────────────────────────────────────

void MaintenanceState::enter(DeviceContext& ctx) {
    Logger::getInstance().info(
        ctx.getDeviceId() + " entering maintenance mode", "MaintenanceState");
}

void MaintenanceState::exit(DeviceContext& ctx) {
    Logger::getInstance().info(
        ctx.getDeviceId() + " maintenance complete", "MaintenanceState");
}

void MaintenanceState::handleEvent(DeviceContext& ctx,
                                    const std::string& event) {
    if (event == "done") {
        ctx.transitionTo(std::make_shared<IdleState>());
    } else {
        Logger::getInstance().warning(
            "Maintenance: only 'done' accepted", "MaintenanceState");
    }
}

DeviceStateType MaintenanceState::getType() const {
    return DeviceStateType::Maintenance;
}
std::string MaintenanceState::getName() const { return "Maintenance"; }

} // namespace iot
