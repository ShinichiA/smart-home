#pragma once

#include "communication/interfaces/IProtocol.h"
#include <memory>

namespace iot {

/**
 * @brief Adapter pattern — adapts different protocols to a uniform interface.
 *
 * Design Patterns: Adapter (Object Adapter)
 * Wraps any IProtocol and adds logging, retry logic, message formatting.
 *
 * Modern C++: std::unique_ptr, delegating constructor, move semantics
 */
class ProtocolAdapter : public IProtocol {
public:
    /**
     * @brief Construct adapter wrapping an existing protocol.
     * @param adaptee The protocol to adapt (takes ownership)
     * @param topicPrefix Prefix to add to all topics automatically
     */
    explicit ProtocolAdapter(std::unique_ptr<IProtocol> adaptee,
                              std::string topicPrefix = "");

    bool connect() override;
    void disconnect() override;
    [[nodiscard]] bool isConnected() const override;

    /**
     * @brief Send with automatic topic prefixing and JSON wrapping.
     */
    bool send(const std::string& topic, const std::string& payload) override;
    std::string receive(const std::string& topic) override;

    [[nodiscard]] std::string getProtocolName() const override;
    void setMessageCallback(MessageCallback callback) override;

    // ── Adapter-specific ─────────────────────────────────────────────
    void setTopicPrefix(const std::string& prefix);
    IProtocol& getAdaptee();

private:
    std::string formatTopic(const std::string& topic) const;
    std::string wrapPayload(const std::string& payload) const;

    std::unique_ptr<IProtocol> adaptee_;
    std::string topicPrefix_;
};

} // namespace iot
