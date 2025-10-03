#include "config/config.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

namespace leizi {

// ========== ConfigValue ==========

bool ConfigValue::asBool() const {
    return value == "true" || value == "1" || value == "yes" || value == "on";
}

int ConfigValue::asInt() const {
    try {
        return std::stoi(value);
    } catch (...) {
        return 0;
    }
}

std::string ConfigValue::asString() const {
    return value;
}

ConfigValue ConfigValue::fromBool(bool b) {
    ConfigValue v;
    v.type = BOOL;
    v.value = b ? "true" : "false";
    return v;
}

ConfigValue ConfigValue::fromInt(int i) {
    ConfigValue v;
    v.type = INT;
    v.value = std::to_string(i);
    return v;
}

ConfigValue ConfigValue::fromString(const std::string& s) {
    ConfigValue v;
    v.type = STRING;
    v.value = s;
    return v;
}

// ========== ConfigManager ==========

ConfigManager::ConfigManager() : loaded_(false) {
    setDefaults();
}

void ConfigManager::setDefaults() {
    // [prompt] 默认值
    config_["prompt"]["show_git"] = ConfigValue::fromBool(true);
    config_["prompt"]["show_time"] = ConfigValue::fromBool(true);
    config_["prompt"]["show_user"] = ConfigValue::fromBool(true);
    config_["prompt"]["colors"] = ConfigValue::fromBool(true);
    config_["prompt"]["symbol"] = ConfigValue::fromString("❯");

    // [completion] 默认值
    config_["completion"]["case_sensitive"] = ConfigValue::fromBool(false);
    config_["completion"]["show_hidden"] = ConfigValue::fromBool(false);

    // [history] 默认值
    config_["history"]["size"] = ConfigValue::fromInt(10000);
    config_["history"]["ignore_duplicates"] = ConfigValue::fromBool(true);
    config_["history"]["ignore_space"] = ConfigValue::fromBool(true);
}

bool ConfigManager::loadConfig(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        return false;
    }

    std::string currentSection;
    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;
        if (!parseLine(line, currentSection)) {
            // 解析错误，继续尝试解析下一行
            // 可以添加错误日志
        }
    }

    loaded_ = true;
    return true;
}

bool ConfigManager::parseLine(const std::string& line, std::string& currentSection) {
    std::string trimmed = trim(line);

    // 跳过空行和注释
    if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') {
        return true;
    }

    // 解析section: [section_name]
    if (trimmed[0] == '[' && trimmed.back() == ']') {
        currentSection = trim(trimmed.substr(1, trimmed.size() - 2));
        return true;
    }

    // 解析 key = value
    size_t eq = trimmed.find('=');
    if (eq == std::string::npos) {
        return false;  // 格式错误
    }

    std::string key = trim(trimmed.substr(0, eq));
    std::string value = trim(trimmed.substr(eq + 1));

    // 移除value的引号
    if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                               (value.front() == '\'' && value.back() == '\''))) {
        value = value.substr(1, value.size() - 2);
    }

    // [aliases] 特殊处理
    if (currentSection == "aliases") {
        aliases_[key] = value;
        return true;
    }

    // 其他section
    if (currentSection.empty()) {
        return false;  // key=value 必须在section下
    }

    // 自动判断类型
    ConfigValue cv;
    if (value == "true" || value == "false" || value == "yes" || value == "no" ||
        value == "on" || value == "off") {
        cv = ConfigValue::fromBool(value == "true" || value == "yes" || value == "on");
    } else {
        // 尝试解析为整数
        try {
            std::stoi(value);
            cv = ConfigValue::fromInt(std::stoi(value));
        } catch (...) {
            // 否则当作字符串
            cv = ConfigValue::fromString(value);
        }
    }

    config_[currentSection][key] = cv;
    return true;
}

std::string ConfigManager::trim(const std::string& str) const {
    size_t start = 0;
    size_t end = str.size();

    while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }

    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        end--;
    }

    return str.substr(start, end - start);
}

std::optional<bool> ConfigManager::getBool(const std::string& section, const std::string& key) const {
    auto secIt = config_.find(section);
    if (secIt == config_.end()) return std::nullopt;

    auto keyIt = secIt->second.find(key);
    if (keyIt == secIt->second.end()) return std::nullopt;

    return keyIt->second.asBool();
}

std::optional<int> ConfigManager::getInt(const std::string& section, const std::string& key) const {
    auto secIt = config_.find(section);
    if (secIt == config_.end()) return std::nullopt;

    auto keyIt = secIt->second.find(key);
    if (keyIt == secIt->second.end()) return std::nullopt;

    return keyIt->second.asInt();
}

std::optional<std::string> ConfigManager::getString(const std::string& section, const std::string& key) const {
    auto secIt = config_.find(section);
    if (secIt == config_.end()) return std::nullopt;

    auto keyIt = secIt->second.find(key);
    if (keyIt == secIt->second.end()) return std::nullopt;

    return keyIt->second.asString();
}

std::optional<std::string> ConfigManager::getAlias(const std::string& name) const {
    auto it = aliases_.find(name);
    if (it == aliases_.end()) return std::nullopt;
    return it->second;
}

std::unordered_map<std::string, std::string> ConfigManager::getAllAliases() const {
    return aliases_;
}

bool ConfigManager::generateDefaultConfig(const std::string& configPath) {
    // 确保目录存在
    size_t lastSlash = configPath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        std::string dir = configPath.substr(0, lastSlash);
        mkdir(dir.c_str(), 0755);
    }

    std::ofstream file(configPath);
    if (!file.is_open()) {
        return false;
    }

    file << "# Leizi Shell Configuration File\n";
    file << "# Generated automatically\n\n";

    file << "[prompt]\n";
    file << "show_git = true\n";
    file << "show_time = true\n";
    file << "show_user = true\n";
    file << "colors = true\n";
    file << "symbol = \"❯\"\n\n";

    file << "[completion]\n";
    file << "case_sensitive = false\n";
    file << "show_hidden = false\n\n";

    file << "[history]\n";
    file << "size = 10000\n";
    file << "ignore_duplicates = true\n";
    file << "ignore_space = true\n\n";

    file << "[aliases]\n";
    file << "ll = \"ls -la\"\n";
    file << "la = \"ls -A\"\n";
    file << "l = \"ls -CF\"\n";
    file << "# Add your custom aliases below\n";

    return true;
}

} // namespace leizi
