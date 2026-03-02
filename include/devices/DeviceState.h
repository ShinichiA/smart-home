#pragma once

#include "devices/interfaces/IDeviceState.h"

namespace iot {

// ── Concrete States ──────────────────────────────────────────────────

class IdleState : public IDeviceState {
public:
    void enter(DeviceContext& ctx) override;
    void exit(DeviceContext& ctx) override;
    void handleEvent(DeviceContext& ctx, const std::string& event) override;
    [[nodiscard]] DeviceStateType getType() const override;
    [[nodiscard]] std::string getName() const override;
};

class ActiveState : public IDeviceState {
public:
    void enter(DeviceContext& ctx) override;
    void exit(DeviceContext& ctx) override;
    void handleEvent(DeviceContext& ctx, const std::string& event) override;
    [[nodiscard]] DeviceStateType getType() const override;
    [[nodiscard]] std::string getName() const override;
};

class ErrorState : public IDeviceState {
public:
    void enter(DeviceContext& ctx) override;
    void exit(DeviceContext& ctx) override;
    void handleEvent(DeviceContext& ctx, const std::string& event) override;
    [[nodiscard]] DeviceStateType getType() const override;
    [[nodiscard]] std::string getName() const override;
};

class MaintenanceState : public IDeviceState {
public:
    void enter(DeviceContext& ctx) override;
    void exit(DeviceContext& ctx) override;
    void handleEvent(DeviceContext& ctx, const std::string& event) override;
    [[nodiscard]] DeviceStateType getType() const override;
    [[nodiscard]] std::string getName() const override;
};

} // namespace iot
