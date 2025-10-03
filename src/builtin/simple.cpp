#include "builtin.h"
#include "../utils/colors.h"
#include <iostream>
#include <unistd.h>
#include <cstdlib>

/**
 * @brief pwd 命令实现
 */
class PwdCommand : public BuiltinCommand {
public:
    std::string getName() const override {
        return "pwd";
    }

    std::string getHelp() const override {
        return "pwd                   Print working directory";
    }

    BuiltinResult execute(const std::vector<std::string>& args, BuiltinContext& context) override {
        BuiltinResult result;
        std::cout << context.currentDirectory << std::endl;
        result.exitCode = 0;
        context.lastExitCode = result.exitCode;
        return result;
    }
};

/**
 * @brief exit 命令实现
 */
class ExitCommand : public BuiltinCommand {
public:
    std::string getName() const override {
        return "exit";
    }

    std::string getHelp() const override {
        return "exit [code]           Exit shell";
    }

    BuiltinResult execute(const std::vector<std::string>& args, BuiltinContext& context) override {
        BuiltinResult result;

        int code = 0;
        if (args.size() > 1) {
            try {
                code = std::stoi(args[1]);
            } catch (...) {
                code = 255;
            }
        }

        context.exitRequested = true;
        result.exitCode = code;
        result.shouldExit = true;
        context.lastExitCode = code;
        return result;
    }
};

/**
 * @brief clear 命令实现
 */
class ClearCommand : public BuiltinCommand {
public:
    std::string getName() const override {
        return "clear";
    }

    std::string getHelp() const override {
        return "clear                 Clear screen";
    }

    BuiltinResult execute(const std::vector<std::string>& args, BuiltinContext& context) override {
        BuiltinResult result;
        std::cout << "\033[2J\033[H";
        result.exitCode = 0;
        context.lastExitCode = result.exitCode;
        return result;
    }
};

// 全局实例
static PwdCommand pwdCommand;
static ExitCommand exitCommand;
static ClearCommand clearCommand;

// 工厂函数
extern "C" BuiltinCommand* createPwdCommand() {
    return &pwdCommand;
}

extern "C" BuiltinCommand* createExitCommand() {
    return &exitCommand;
}

extern "C" BuiltinCommand* createClearCommand() {
    return &clearCommand;
}
