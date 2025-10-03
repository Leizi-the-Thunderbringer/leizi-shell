#include "builtin.h"
#include "../utils/colors.h"
#include "../syntax/highlighter.h"
#include "../builtin/builtin_manager.h"

#include <iostream>

/**
 * @brief highlight 命令 - 演示语法高亮功能
 *
 * 用法: highlight <command>
 */
class HighlightCommand : public BuiltinCommand {
public:
    std::string getName() const override {
        return "highlight";
    }

    std::string getHelp() const override {
        return "highlight <command>   Demonstrate syntax highlighting";
    }

    BuiltinResult execute(const std::vector<std::string>& args, BuiltinContext& context) override {
        BuiltinResult result;

        if (args.size() < 2) {
            std::cout << "Usage: highlight <command>" << std::endl;
            std::cout << "Examples:" << std::endl;
            std::cout << "  highlight echo hello world" << std::endl;
            std::cout << "  highlight ls -la | grep test > file.txt" << std::endl;
            std::cout << "  highlight export PATH=/usr/bin:$PATH" << std::endl;
            result.exitCode = 0;
            context.lastExitCode = result.exitCode;
            return result;
        }

        // 重建命令字符串（从第二个参数开始）
        std::string command;
        for (size_t i = 1; i < args.size(); ++i) {
            if (i > 1) command += " ";
            command += args[i];
        }

        // 创建临时高亮器（使用简单的内建命令列表）
        std::vector<std::string> builtins = {
            "cd", "pwd", "exit", "clear", "help", "version",
            "export", "unset", "env", "array", "history",
            "exec", "jobs", "fg", "bg", "highlight"
        };
        leizi::SyntaxHighlighter highlighter(builtins);

        // 应用高亮
        std::string highlighted = highlighter.highlight(command);

        std::cout << Color::DIM << "Original:    " << Color::RESET << command << std::endl;
        std::cout << Color::DIM << "Highlighted: " << Color::RESET << highlighted << std::endl;

        result.exitCode = 0;
        context.lastExitCode = result.exitCode;
        return result;
    }
};

// 自动注册到全局工厂
extern "C" BuiltinCommand* createHighlightCommand() {
    return new HighlightCommand();
}

