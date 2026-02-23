#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

namespace iot {

/**
 * @brief State pattern for device lifecycle management.
 *
 * Design Patterns: State
 * States: Idle → Active → Error → Maintenance
 *
 * Modern C++: enum class, std::unordered_map, std::function,
 *             shared_ptr for state transitions
 */
enum class DeviceStateType : uint8_t {
    Idle        = 0,
    Active      = 1,
    Error       = 2,
    Maintenance = 3
};

inline std::string stateToString(DeviceStateType state) {
    switch (state) {
        case DeviceStateType::Idle:        return "Idle";
        case DeviceStateType::Active:      return "Active";
        case DeviceStateType::Error:       return "Error";
        case DeviceStateType::Maintenance: return "Maintenance";
    }
    return "Unknown";
}

// Forward declaration
class DeviceContext;

/**
 * @brief Abstract state interface.
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
