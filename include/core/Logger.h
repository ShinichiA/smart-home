#pragma once

#include <string>
#include <mutex>
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <thread>
#include <memory>
#include <functional>

namespace iot {

// ─── Log Level Enum Class ───────────────────────────────────────────
enum class LogLevel : uint8_t {
    DEBUG   = 0,
    INFO    = 1,
    WARNING = 2,
    ERROR   = 3
};

/**
 * @brief Thread-safe Singleton Logger with multiple output targets.
 *
 * Design Patterns: Singleton
 * Modern C++: enum class, std::mutex, RAII lock_guard, chrono,
 *             deleted copy/move, static local (Meyers' Singleton)
 */
class Logger {
public:
    // ── Meyers' Singleton ────────────────────────────────────────────
    static Logger& getInstance();

    // ── Deleted copy & move (Singleton guarantee) ────────────────────
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    // ── Configuration ────────────────────────────────────────────────
    void setLogLevel(LogLevel level);
    void setLogFile(const std::string& filepath);
    void enableConsoleOutput(bool enable);

    // ── Core logging API ─────────────────────────────────────────────
    void log(LogLevel level, const std::string& message,
             const std::string& source = "");

    // ── Convenience methods ──────────────────────────────────────────
    void debug(const std::string& msg, const std::string& src = "");
    void info(const std::string& msg, const std::string& src = "");
    void warning(const std::string& msg, const std::string& src = "");
    void error(const std::string& msg, const std::string& src = "");

    // ── Custom log sink (callback) ───────────────────────────────────
    using LogCallback = std::function<void(LogLevel, const std::string&)>;
    void setCustomSink(LogCallback callback);

private:
    Logger();
    ~Logger();

    std::string levelToString(LogLevel level) const;
    std::string getCurrentTimestamp() const;
    std::string getThreadId() const;

    LogLevel        minLevel_{LogLevel::DEBUG};
    bool            consoleEnabled_{true};
    std::ofstream   logFile_;
    std::mutex      mutex_;
    LogCallback     customSink_;
};

} // namespace iot
