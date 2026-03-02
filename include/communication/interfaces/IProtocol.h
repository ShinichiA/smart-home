#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace iot {

/**
 * @brief Abstract interface for communication protocols.
 * SOLID: Dependency Inversion — depend on abstraction, not concrete MQTT/HTTP.
 */
class IProtocol {
public:
    virtual ~IProtocol() = default;

    // ── Connection lifecycle ─────────────────────────────────────────
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    [[nodiscard]] virtual bool isConnected() const = 0;

    // ── Data transfer ────────────────────────────────────────────────
    virtual bool send(const std::string& topic, const std::string& payload) = 0;
    virtual std::string receive(const std::string& topic) = 0;

    // ── Protocol info ────────────────────────────────────────────────
    [[nodiscard]] virtual std::string getProtocolName() const = 0;

    // ── Async message callback ───────────────────────────────────────
    using MessageCallback = std::function<void(const std::string& topic,
                                                const std::string& payload)>;
    virtual void setMessageCallback(MessageCallback callback) = 0;

protected:
    IProtocol() = default;
    IProtocol(const IProtocol&) = default;
    IProtocol& operator=(const IProtocol&) = default;
    IProtocol(IProtocol&&) = default;
    IProtocol& operator=(IProtocol&&) = default;
};

} // namespace iot
