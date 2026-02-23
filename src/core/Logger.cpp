#include "core/Logger.h"

namespace iot {

// ── Meyers' Singleton (thread-safe in C++11+) ────────────────────────
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger()  = default;

Logger::~Logger() {
    if (logFile_.is_open()) {
        logFile_.close();
    }
}

// ── Configuration ────────────────────────────────────────────────────

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    minLevel_ = level;
}

void Logger::setLogFile(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_.is_open()) {
        logFile_.close();
    }
    logFile_.open(filepath, std::ios::app);
}

void Logger::enableConsoleOutput(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    consoleEnabled_ = enable;
}

void Logger::setCustomSink(LogCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    customSink_ = std::move(callback);
}

// ── Core logging ─────────────────────────────────────────────────────

void Logger::log(LogLevel level, const std::string& message,
                 const std::string& source) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (level < minLevel_) return;

    std::ostringstream oss;
    oss << "[" << getCurrentTimestamp() << "] "
        << "[" << levelToString(level) << "] "
        << "[T:" << getThreadId() << "] ";

    if (!source.empty()) {
        oss << "[" << source << "] ";
    }
    oss << message;

    const auto formatted = oss.str();

    if (consoleEnabled_) {
        // Color codes for terminal
        switch (level) {
            case LogLevel::DEBUG:   std::cout << "\033[36m";  break; // Cyan
            case LogLevel::INFO:    std::cout << "\033[32m";  break; // Green
            case LogLevel::WARNING: std::cout << "\033[33m";  break; // Yellow
            case LogLevel::ERROR:   std::cout << "\033[31m";  break; // Red
        }
        std::cout << formatted << "\033[0m" << std::endl;
    }

    if (logFile_.is_open()) {
        logFile_ << formatted << std::endl;
    }

    if (customSink_) {
        customSink_(level, formatted);
    }
}

// ── Convenience methods ──────────────────────────────────────────────

void Logger::debug(const std::string& msg, const std::string& src) {
    log(LogLevel::DEBUG, msg, src);
}

void Logger::info(const std::string& msg, const std::string& src) {
    log(LogLevel::INFO, msg, src);
}

void Logger::warning(const std::string& msg, const std::string& src) {
    log(LogLevel::WARNING, msg, src);
}

void Logger::error(const std::string& msg, const std::string& src) {
    log(LogLevel::ERROR, msg, src);
}

// ── Helpers ──────────────────────────────────────────────────────────

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG  ";
        case LogLevel::INFO:    return "INFO   ";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR  ";
    }
    return "UNKNOWN";
}

std::string Logger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
        << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::getThreadId() const {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    return oss.str();
}

} // namespace iot
