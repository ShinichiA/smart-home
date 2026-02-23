#pragma once

#include "communication/IProtocol.h"
#include <atomic>
#include <map>
#include <mutex>

namespace iot {

/**
 * @brief Simulated HTTP REST client implementing IProtocol.
 *
 * Maps topic to REST endpoint, send() = POST, receive() = GET.
 * Modern C++: std::map, structured bindings, atomic
 */
class HttpClient : public IProtocol {
public:
    HttpClient(std::string baseUrl, int port,
               std::string apiKey, int timeoutMs = 5000);
    ~HttpClient() override;

    bool connect() override;
    void disconnect() override;
    [[nodiscard]] bool isConnected() const override;

    bool send(const std::string& endpoint, const std::string& payload) override;
    std::string receive(const std::string& endpoint) override;

    [[nodiscard]] std::string getProtocolName() const override;
    void setMessageCallback(MessageCallback callback) override;

    // HTTP-specific
    void setRetryCount(int count);

private:
    std::string     baseUrl_;
    int             port_;
    std::string     apiKey_;
    int             timeoutMs_;
    int             retryCount_{3};
    std::atomic<bool> connected_{false};

    MessageCallback messageCallback_;
    std::map<std::string, std::string> lastResponses_;
    std::mutex responseMutex_;
};

} // namespace iot
