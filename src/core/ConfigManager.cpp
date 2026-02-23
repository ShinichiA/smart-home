#include "core/ConfigManager.h"
#include "core/Logger.h"
#include <iostream>
#include <algorithm>
#include <cctype>

namespace iot {

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager& ConfigManager::loadFromFile(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ifstream file(filepath);
    if (!file.is_open()) {
        Logger::getInstance().error(
            "Failed to open config file: " + filepath, "ConfigManager");
        return *this;
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::string content = oss.str();

    parseJsonContent(content);

    Logger::getInstance().info(
        "Loaded config from: " + filepath +
        " (" + std::to_string(values_.size()) + " entries)",
        "ConfigManager");

    return *this;
}

ConfigManager& ConfigManager::set(const std::string& key, ConfigValue value) {
    std::lock_guard<std::mutex> lock(mutex_);
    values_[key] = std::move(value);
    return *this;
}

bool ConfigManager::hasKey(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return values_.find(key) != values_.end();
}

void ConfigManager::printAll() const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto& logger = Logger::getInstance();
    logger.info("=== Configuration Dump ===", "ConfigManager");

    for (const auto& [key, value] : values_) {
        std::string valStr;
        std::visit([&valStr](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, bool>) {
                valStr = v ? "true" : "false";
            } else if constexpr (std::is_same_v<T, std::string>) {
                valStr = "\"" + v + "\"";
            } else {
                valStr = std::to_string(v);
            }
        }, value);
        logger.info("  " + key + " = " + valStr, "ConfigManager");
    }
}

// ── Simple recursive JSON parser (no external lib) ──────────────────

std::string ConfigManager::trim(const std::string& str) const {
    auto start = str.find_first_not_of(" \t\n\r");
    auto end = str.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

void ConfigManager::parseJsonContent(const std::string& content,
                                     const std::string& prefix) {
    // Minimal JSON key-value parser for flat & nested objects
    // Handles: string, number (int/double), bool values
    std::string cleaned;
    bool inString = false;
    for (size_t i = 0; i < content.size(); ++i) {
        char c = content[i];
        if (c == '"' && (i == 0 || content[i - 1] != '\\')) {
            inString = !inString;
        }
        if (!inString && (c == '\n' || c == '\r')) continue;
        cleaned += c;
    }

    size_t pos = 0;
    auto skipWhitespace = [&]() {
        while (pos < cleaned.size() && std::isspace(static_cast<unsigned char>(cleaned[pos])))
            ++pos;
    };

    auto readString = [&]() -> std::string {
        if (cleaned[pos] != '"') return "";
        ++pos;
        std::string result;
        while (pos < cleaned.size() && cleaned[pos] != '"') {
            if (cleaned[pos] == '\\' && pos + 1 < cleaned.size()) {
                result += cleaned[pos + 1];
                pos += 2;
            } else {
                result += cleaned[pos++];
            }
        }
        if (pos < cleaned.size()) ++pos; // skip closing "
        return result;
    };

    // Find object start
    while (pos < cleaned.size() && cleaned[pos] != '{') ++pos;
    if (pos >= cleaned.size()) return;
    ++pos; // skip {

    int depth = 1;
    while (pos < cleaned.size() && depth > 0) {
        skipWhitespace();
        if (pos >= cleaned.size()) break;

        if (cleaned[pos] == '}') { --depth; ++pos; continue; }
        if (cleaned[pos] == ',') { ++pos; continue; }

        // Read key
        if (cleaned[pos] != '"') { ++pos; continue; }
        std::string key = readString();
        std::string fullKey = prefix.empty() ? key : prefix + "." + key;

        skipWhitespace();
        if (pos < cleaned.size() && cleaned[pos] == ':') ++pos;
        skipWhitespace();

        if (pos >= cleaned.size()) break;

        // Read value
        if (cleaned[pos] == '{') {
            // Nested object — find matching }
            int nestedDepth = 0;
            size_t start = pos;
            bool inStr = false;
            while (pos < cleaned.size()) {
                if (cleaned[pos] == '"' && (pos == 0 || cleaned[pos-1] != '\\'))
                    inStr = !inStr;
                if (!inStr) {
                    if (cleaned[pos] == '{') ++nestedDepth;
                    if (cleaned[pos] == '}') {
                        --nestedDepth;
                        if (nestedDepth == 0) { ++pos; break; }
                    }
                }
                ++pos;
            }
            parseJsonContent(cleaned.substr(start, pos - start), fullKey);
        } else if (cleaned[pos] == '"') {
            values_[fullKey] = readString();
        } else {
            // Number or bool
            size_t start = pos;
            while (pos < cleaned.size() && cleaned[pos] != ',' &&
                   cleaned[pos] != '}' && !std::isspace(static_cast<unsigned char>(cleaned[pos])))
                ++pos;
            std::string val = trim(cleaned.substr(start, pos - start));

            if (val == "true")       values_[fullKey] = true;
            else if (val == "false") values_[fullKey] = false;
            else if (val.find('.') != std::string::npos) {
                try { values_[fullKey] = std::stod(val); }
                catch (...) { values_[fullKey] = val; }
            } else {
                try { values_[fullKey] = std::stoi(val); }
                catch (...) { values_[fullKey] = val; }
            }
        }
    }
}

} // namespace iot
