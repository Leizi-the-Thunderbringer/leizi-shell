#ifndef LEIZI_SHELL_VARIABLES_H
#define LEIZI_SHELL_VARIABLES_H

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <functional>
#include <memory>

namespace leizi {

// 变量类型定义
using VariableValue = std::variant<
    std::string,                    // 字符串变量
    std::vector<std::string>,       // 数组变量
    int,                            // 整数变量
    double                          // 浮点数变量
>;

// 变量标志
enum class VariableFlags : uint8_t {
    NONE = 0,
    EXPORTED = 1 << 0,     // 导出到子进程
    READONLY = 1 << 1,     // 只读变量
    LOCAL = 1 << 2,        // 局部变量
    SPECIAL = 1 << 3       // 特殊变量（动态计算）
};

// 变量结构
struct Variable {
    VariableValue value;
    uint8_t flags = static_cast<uint8_t>(VariableFlags::NONE);

    Variable() = default;
    Variable(const VariableValue& val, bool exported = false);

    bool isExported() const;
    bool isReadonly() const;
    bool isLocal() const;
    bool isSpecial() const;

    std::string toString() const;
    std::vector<std::string> toArray() const;
};

// 变量管理器
class VariableManager {
private:
    std::unordered_map<std::string, Variable> variables_;
    std::unordered_map<std::string, std::function<std::string()>> specialVars_;

    // 特殊变量处理
    void initializeSpecialVariables();
    std::string getSpecialVariable(const std::string& name) const;

public:
    VariableManager();

    // 设置变量
    void set(const std::string& name, const VariableValue& value, bool exported = false);
    void setReadonly(const std::string& name, const VariableValue& value);
    void setLocal(const std::string& name, const VariableValue& value);

    // 获取变量
    Variable* get(const std::string& name);
    const Variable* get(const std::string& name) const;
    std::string getValue(const std::string& name) const;

    // 删除变量
    bool unset(const std::string& name);

    // 导出变量
    void exportVar(const std::string& name);
    void unexportVar(const std::string& name);

    // 变量展开
    std::string expand(const std::string& text) const;
    std::string expandVariable(const std::string& varName) const;

    // 获取所有变量
    std::unordered_map<std::string, Variable> getAllVariables() const;
    std::unordered_map<std::string, std::string> getExportedVariables() const;

    // 注册特殊变量
    void registerSpecialVar(const std::string& name, std::function<std::string()> getter);
};

} // namespace leizi

#endif // LEIZI_SHELL_VARIABLES_H