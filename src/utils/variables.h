#pragma once

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// 表示 shell 变量的类型。
enum class VarType {
    STRING,
    ARRAY,
    INTEGER,
    READONLY
};

// 变量值封装，提供简单的类型转换能力。
struct Variable {
    VarType type = VarType::STRING;
    std::string stringValue;
    std::vector<std::string> arrayValue;
    int intValue = 0;
    bool isReadonly = false;

    Variable() = default;
    explicit Variable(const std::string& str, bool readonly = false);
    explicit Variable(const std::vector<std::string>& arr, bool readonly = false);
    explicit Variable(int value, bool readonly = false);

    std::string toString() const;
};

// 管理 shell 变量的容器，支持设置、查询与展开。
class VariableManager {
public:
    VariableManager() = default;

    Variable& set(const std::string& name, const Variable& value);
    Variable& setString(const std::string& name, const std::string& value, bool readonly = false);
    Variable& setArray(const std::string& name, const std::vector<std::string>& values, bool readonly = false);
    Variable& setInteger(const std::string& name, int value, bool readonly = false);

    const Variable* get(const std::string& name) const;
    Variable* get(const std::string& name);

    bool erase(const std::string& name);
    bool contains(const std::string& name) const;

    using Resolver = std::function<std::optional<std::string>(const std::string&)>;
    std::string expand(const std::string& input, const Resolver& resolver = {}) const;

private:
    std::unordered_map<std::string, Variable> variables_;
};

