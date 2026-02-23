#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <optional>
#include <mutex>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <memory>

namespace iot {

// ─── Multi-type config value using std::variant ──────────────────────
using ConfigValue = std::variant<bool, int, double, std::string>;

/**
 * @brief Thread-safe Singleton ConfigManager with Builder-style loading.
 *
 * Design Patterns: Singleton, Builder (fluent config loading)
 * Modern C++: std::variant, std::optional, std::unordered_map,
 *             structured bindings, if constexpr
 */
class ConfigManager {
public:
    // ── Singleton ────────────────────────────────────────────────────
    static ConfigManager& getInstance();

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = delete;
    ConfigManager& operator=(ConfigManager&&) = delete;

    // ── Builder-style loading (returns *this for chaining) ───────────
    ConfigManager& loadFromFile(const std::string& filepath);
    ConfigManager& set(const std::string& key, ConfigValue value);

    // ── Type-safe getters using std::optional ────────────────────────
    template<typename T>
    std::optional<T> get(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = values_.find(key);
        if (it == values_.end()) return std::nullopt;

        // Use std::get_if for safe extraction
        if (auto val = std::get_if<T>(&it->second)) {
            return *val;
        }
        return std::nullopt;
    }

    // ── Get with default value ───────────────────────────────────────
    template<typename T>
    T getOrDefault(const std::string& key, T defaultValue) const {
        auto val = get<T>(key);
        return val.value_or(std::move(defaultValue));
    }

    bool hasKey(const std::string& key) const;
    void printAll() const;

private:
    ConfigManager() = default;
    ~ConfigManager() = default;

    // Simple JSON-like parser (no external dependency)
    void parseJsonContent(const std::string& content,
                          const std::string& prefix = "");
    std::string trim(const std::string& str) const;

    std::unordered_map<std::string, ConfigValue> values_;
    mutable std::mutex mutex_;
};

} // namespace iot
