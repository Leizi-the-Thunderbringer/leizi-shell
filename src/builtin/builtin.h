#ifndef LEIZI_BUILTIN_BUILTIN_H
#define LEIZI_BUILTIN_BUILTIN_H

#include <string>
#include <vector>
#include "builtin_context.h"

/**
 * @brief 内建命令执行结果
 */
struct BuiltinResult {
    int exitCode = 0;           // 退出码
    std::string output;         // 输出内容（如果有）
    bool shouldExit = false;    // 是否应该退出 shell
};

/**
 * @brief 内建命令基类
 *
 * 所有内建命令都应该继承这个基类并实现 execute() 方法
 */
class BuiltinCommand {
public:
    virtual ~BuiltinCommand() = default;

    /**
     * @brief 执行命令
     * @param args 命令参数列表（包含命令名本身）
     * @param context 执行上下文
     * @return 执行结果
     */
    virtual BuiltinResult execute(const std::vector<std::string>& args, BuiltinContext& context) = 0;

    /**
     * @brief 获取命令名称
     */
    virtual std::string getName() const = 0;

    /**
     * @brief 获取命令帮助信息
     */
    virtual std::string getHelp() const = 0;
};

#endif // LEIZI_BUILTIN_BUILTIN_H
