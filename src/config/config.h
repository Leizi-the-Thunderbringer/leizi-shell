#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <vector>

namespace leizi {

// 配置项值类型
struct ConfigValue {
    enum Type { BOOL, INT, STRING };
    Type type;
    std::string value;

    bool asBool() const;
    int asInt() const;
    std::string asString() const;

    static ConfigValue fromBool(bool b);
    static ConfigValue fromInt(int i);
    static ConfigValue fromString(const std::string& s);
};

// Shell配置管理器
class ConfigManager {
public:
    ConfigManager();

    // 加载配置文件
    bool loadConfig(const std::string& configPath);

    // 生成默认配置文件
    bool generateDefaultConfig(const std::string& configPath);

    // 获取配置值
    std::optional<bool> getBool(const std::string& section, const std::string& key) const;
    std::optional<int> getInt(const std::string& section, const std::string& key) const;
    std::optional<std::string> getString(const std::string& section, const std::string& key) const;

    // 获取alias
    std::optional<std::string> getAlias(const std::string& name) const;
    std::unordered_map<std::string, std::string> getAllAliases() const;

    // 检查配置是否加载
    bool isLoaded() const { return loaded_; }

private:
    bool loaded_;
    // section -> (key -> value)
    std::unordered_map<std::string, std::unordered_map<std::string, ConfigValue>> config_;
    std::unordered_map<std::string, std::string> aliases_;

    void setDefaults();
    bool parseLine(const std::string& line, std::string& currentSection);
    std::string trim(const std::string& str) const;
};

} // namespace leizi
