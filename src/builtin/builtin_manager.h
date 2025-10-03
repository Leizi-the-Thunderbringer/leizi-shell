#ifndef LEIZI_BUILTIN_MANAGER_H
#define LEIZI_BUILTIN_MANAGER_H

#include "builtin.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief 内建命令管理器
 *
 * 负责注册和调度所有内建命令
 */
class BuiltinManager {
private:
    std::unordered_map<std::string, BuiltinCommand*> commands;

public:
    BuiltinManager();
    ~BuiltinManager() = default;

    /**
     * @brief 注册内建命令
     */
    void registerCommand(BuiltinCommand* command);

    /**
     * @brief 检查是否为内建命令
     */
    bool isBuiltin(const std::string& name) const;

    /**
     * @brief 执行内建命令
     */
    BuiltinResult execute(const std::vector<std::string>& args, BuiltinContext& context);

    /**
     * @brief 获取所有内建命令名称列表
     */
    std::vector<std::string> getCommandNames() const;
};

#endif // LEIZI_BUILTIN_MANAGER_H
