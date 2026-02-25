#pragma once

#include "sensors/interfaces/ISensor.h"
#include "core/Logger.h"
#include <chrono>
#include <random>
#include <atomic>

namespace iot {

/**
 * @brief Abstract base sensor implementing Template Method pattern.
 *
 * Design Patterns: Template Method
 *   - read() defines the skeleton: readRawValue() → applyCalibration() → validate()
 *   - Subclasses override the hook methods
 *
 * Modern C++: RAII, std::atomic, chrono, random engine, override/final
 */
class BaseSensor : public ISensor {
public:
    BaseSensor(std::string name, SensorType type, int pin);
    ~BaseSensor() override;

    // ── ISensor interface ────────────────────────────────────────────
    [[nodiscard]] std::string getName() const override;
    [[nodiscard]] SensorType  getType() const override;
    [[nodiscard]] bool        isInitialized() const override;

    bool initialize() override;
    void shutdown() override;
    void calibrate(double offset) override;

    /**
     * @brief Template Method — defines the algorithm skeleton.
     * Calls: readRawValue() → applyCalibration() → validateReading()
     */
    SensorReading read() override final;

protected:
    // ── Hook methods for subclass customization ──────────────────────
    virtual double readRawValue() = 0;
    virtual double applyCalibration(double raw) const;
    virtual bool   validateReading(double value) const;
    virtual std::string getUnit() const = 0;

    // Subclass hooks for init/shutdown
    virtual bool onInitialize() { return true; }
    virtual void onShutdown() {}

    // ── Utility for simulation ───────────────────────────────────────
    double generateRandomValue(double min, double max);

    std::string     name_;
    SensorType      type_;
    int             pin_;
    double          calibrationOffset_{0.0};
    std::atomic<bool> initialized_{false};

    // Random engine per sensor (thread-local not needed, one per instance)
    std::mt19937    rng_;
};

} // namespace iot
