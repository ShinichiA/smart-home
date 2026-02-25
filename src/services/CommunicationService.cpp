#include "services/CommunicationService.h"

namespace iot {

CommunicationService::CommunicationService() = default;

CommunicationService::~CommunicationService() {
    shutdown();
}

bool CommunicationService::initialize() {
    auto& log = Logger::getInstance();
    log.info("Initializing CommunicationService...", "CommService");

    // Abstract Factory: create protocol from config
    protocol_ = CommunicationFactory::createFromConfig();
    if (!protocol_) {
        log.error("Failed to create protocol", "CommService");
        return false;
    }

    // Connect
    if (!protocol_->connect()) {
        log.error("Failed to connect protocol", "CommService");
        return false;
    }

    // Set message callback for ACK logging
    protocol_->setMessageCallback(
        [&log](const std::string& topic, const std::string& payload) {
            log.debug("📨 Backend ACK [" + topic + "] len=" +
                      std::to_string(payload.size()), "CommService");
        });

    log.info("CommunicationService initialized via " +
             protocol_->getProtocolName(), "CommService");
    return true;
}

void CommunicationService::startListening() {
    if (listening_) return;

    auto& log = Logger::getInstance();

    // Subscribe to sensor.reading events → auto-send to backend
    sensorSubId_ = EventBus::getInstance().subscribe<SensorEvent>(
        "sensor.reading",
        [this, &log](const SensorEvent& event) {
            if (!protocol_ || !protocol_->isConnected()) return;

            // Build a minimal SensorReading from event for JSON
            SensorReading reading;
            reading.sensorName    = event.sensorName;
            reading.processedValue = event.value;
            reading.timestampMs   = event.timestampMs;
            reading.isValid       = true;

            auto json = readingToJson(reading);
            protocol_->send(event.sensorName, json);
        });

    listening_ = true;
    log.info("CommunicationService listening for sensor events",
             "CommService");
}

void CommunicationService::shutdown() {
    auto& log = Logger::getInstance();

    if (listening_) {
        EventBus::getInstance().unsubscribe("sensor.reading",
                                             sensorSubId_);
        listening_ = false;
    }

    if (protocol_) {
        protocol_->disconnect();
        log.info("CommunicationService disconnected", "CommService");
    }
}

bool CommunicationService::sendReading(const SensorReading& reading) {
    if (!protocol_ || !protocol_->isConnected()) return false;
    auto json = readingToJson(reading);
    return protocol_->send(reading.sensorName, json);
}

bool CommunicationService::isConnected() const {
    return protocol_ && protocol_->isConnected();
}

std::string CommunicationService::getProtocolName() const {
    return protocol_ ? protocol_->getProtocolName() : "None";
}

std::string CommunicationService::readingToJson(
    const SensorReading& r) {
    std::string json = "{";
    json += R"("sensor":")" + r.sensorName + "\",";
    json += R"("type":")" + sensorTypeToString(r.type) + "\",";
    json += R"("raw":)" + std::to_string(r.rawValue) + ",";
    json += R"("value":)" + std::to_string(r.processedValue) + ",";
    json += R"("valid":)" + std::string(r.isValid ? "true" : "false") + ",";
    json += R"("timestamp":)" + std::to_string(r.timestampMs);
    if (r.unit.has_value()) {
        json += R"(,"unit":")" + r.unit.value() + "\"";
    }
    json += "}";
    return json;
}

} // namespace iot
