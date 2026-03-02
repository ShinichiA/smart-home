#pragma once

#include "devices/interfaces/DeviceTypes.h"
#include <string>
#include <memory>

namespace iot {

// Forward declaration
class DeviceContext;

/**
 * @brief Abstract state interface for device lifecycle.
 *
 * Design Patterns: State
 * States: Idle → Active → Error → Maintenance
 */
class IDeviceState {
public:
    virtual ~IDeviceState() = default;

    virtual void enter(DeviceContext& ctx) = 0;
    virtual void exit(DeviceContext& ctx) = 0;
    virtual void handleEvent(DeviceContext& ctx,
                             const std::string& event) = 0;

    [[nodiscard]] virtual DeviceStateType getType() const = 0;
    [[nodiscard]] virtual std::string getName() const = 0;

protected:
    IDeviceState() = default;
    IDeviceState(const IDeviceState&) = default;
    IDeviceState& operator=(const IDeviceState&) = default;
    IDeviceState(IDeviceState&&) = default;
    IDeviceState& operator=(IDeviceState&&) = default;
};

/**
 * @brief Context that holds the current state and manages transitions.
 */
class DeviceContext {
public:
    explicit DeviceContext(const std::string& deviceId);

    void transitionTo(std::shared_ptr<IDeviceState> newState);
    void handleEvent(const std::string& event);

    [[nodiscard]] DeviceStateType getCurrentStateType() const;
    [[nodiscard]] std::string getDeviceId() const;

private:
    std::string deviceId_;
    std::shared_ptr<IDeviceState> currentState_;
};

} // namespace iot
