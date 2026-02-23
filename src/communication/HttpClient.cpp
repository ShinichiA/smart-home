#include "communication/HttpClient.h"
#include "core/Logger.h"

namespace iot {

HttpClient::HttpClient(std::string baseUrl, int port,
                        std::string apiKey, int timeoutMs)
    : baseUrl_(std::move(baseUrl))
    , port_(port)
    , apiKey_(std::move(apiKey))
    , timeoutMs_(timeoutMs)
{}

HttpClient::~HttpClient() {
    if (connected_.load()) {
        disconnect();
    }
}

bool HttpClient::connect() {
    auto& log = Logger::getInstance();
    log.info("HTTP connecting to " + baseUrl_ + ":" +
             std::to_string(port_), "HttpClient");

    // Simulate TLS handshake
    connected_.store(true);
    log.info("HTTP connected (timeout=" + std::to_string(timeoutMs_) +
             "ms, retries=" + std::to_string(retryCount_) + ")", "HttpClient");
    return true;
}

void HttpClient::disconnect() {
    Logger::getInstance().info("HTTP disconnecting", "HttpClient");
    connected_.store(false);
}

bool HttpClient::isConnected() const {
    return connected_.load();
}

bool HttpClient::send(const std::string& endpoint, const std::string& payload) {
    if (!connected_.load()) {
        Logger::getInstance().error(
            "HTTP not connected, cannot POST", "HttpClient");
        return false;
    }

    Logger::getInstance().debug(
        "HTTP POST " + baseUrl_ + endpoint +
        " payload_size=" + std::to_string(payload.size()) +
        " auth=Bearer[...]", "HttpClient");

    // Simulate response
    {
        std::lock_guard<std::mutex> lock(responseMutex_);
        lastResponses_[endpoint] = R"({"status":"ok","code":200})";
    }

    if (messageCallback_) {
        messageCallback_(endpoint, R"({"status":"accepted"})");
    }

    return true;
}

std::string HttpClient::receive(const std::string& endpoint) {
    if (!connected_.load()) return "";

    Logger::getInstance().debug(
        "HTTP GET " + baseUrl_ + endpoint, "HttpClient");

    std::lock_guard<std::mutex> lock(responseMutex_);
    auto it = lastResponses_.find(endpoint);
    if (it != lastResponses_.end()) {
        return it->second;
    }
    return R"({"status":"no_data"})";
}

std::string HttpClient::getProtocolName() const {
    return "HTTP";
}

void HttpClient::setMessageCallback(MessageCallback callback) {
    messageCallback_ = std::move(callback);
}

void HttpClient::setRetryCount(int count) {
    retryCount_ = count;
}

} // namespace iot
