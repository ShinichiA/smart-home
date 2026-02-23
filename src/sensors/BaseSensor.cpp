#include "sensors/BaseSensor.h"

namespace iot {

BaseSensor::BaseSensor(std::string name, SensorType type, int pin)
    : name_(std::move(name))
    , type_(type)
    , pin_(pin)
    , rng_(std::random_device{}())
{
    Logger::getInstance().debug(
        "BaseSensor created: " + name_ + " on pin " + std::to_string(pin_),
        "BaseSensor");
}

BaseSensor::~BaseSensor() {
    if (initialized_.load()) {
        shutdown();
    }
}

std::string BaseSensor::getName() const { return name_; }
SensorType  BaseSensor::getType() const { return type_; }
bool        BaseSensor::isInitialized() const { return initialized_.load(); }

bool BaseSensor::initialize() {
    if (initialized_.load()) {
        Logger::getInstance().warning(
            name_ + " already initialized", "BaseSensor");
        return true;
    }

    Logger::getInstance().info(
        "Initializing sensor: " + name_, "BaseSensor");

    if (!onInitialize()) {
        Logger::getInstance().error(
            "Failed to initialize: " + name_, "BaseSensor");
        return false;
    }

    initialized_.store(true);
    Logger::getInstance().info(
        name_ + " initialized successfully", "BaseSensor");
    return true;
}

void BaseSensor::shutdown() {
    if (!initialized_.load()) return;

    Logger::getInstance().info(
        "Shutting down sensor: " + name_, "BaseSensor");

    onShutdown();
    initialized_.store(false);
}

void BaseSensor::calibrate(double offset) {
    calibrationOffset_ = offset;
    Logger::getInstance().info(
        name_ + " calibrated with offset: " + std::to_string(offset),
        "BaseSensor");
}

// ── Template Method implementation ───────────────────────────────────

SensorReading BaseSensor::read() {
    SensorReading reading;
    reading.sensorName  = name_;
    reading.type        = type_;

    auto now = std::chrono::system_clock::now();
    reading.timestampMs = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count());

    if (!initialized_.load()) {
        Logger::getInstance().error(
            "Attempt to read uninitialized sensor: " + name_,
            "BaseSensor");
        reading.isValid = false;
        reading.rawValue = 0.0;
        reading.processedValue = 0.0;
        return reading;
    }

    // Step 1: Read raw value (subclass hook)
    reading.rawValue = readRawValue();

    // Step 2: Apply calibration (subclass hook)
    reading.processedValue = applyCalibration(reading.rawValue);

    // Step 3: Validate (subclass hook)
    reading.isValid = validateReading(reading.processedValue);

    // Step 4: Set unit
    reading.unit = getUnit();

    return reading;
}

double BaseSensor::applyCalibration(double raw) const {
    return raw + calibrationOffset_;
}

bool BaseSensor::validateReading(double /*value*/) const {
    return true;  // Default: accept all readings
}

double BaseSensor::generateRandomValue(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng_);
}

} // namespace iot
