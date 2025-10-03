#include "builtin.h"
#include "../utils/colors.h"
#include <iostream>

/**
 * @brief echo 命令实现
 */
class EchoCommand : public BuiltinCommand {
public:
    std::string getName() const override {
        return "echo";
    }

    std::string getHelp() const override {
        return "echo [-n] text         Print text";
    }

    BuiltinResult execute(const std::vector<std::string>& args, BuiltinContext& context) override {
        BuiltinResult result;

        bool newline = true;
        size_t start = 1;

        // 处理 -n 选项
        if (args.size() > 1 && args[1] == "-n") {
            newline = false;
            start = 2;
        }

        for (size_t i = start; i < args.size(); ++i) {
            if (i > start) std::cout << " ";
            std::cout << context.expandVariables(args[i]);
        }
        if (newline) std::cout << std::endl;

        result.exitCode = 0;
        context.lastExitCode = result.exitCode;
        return result;
    }
};

// 全局实例
static EchoCommand echoCommand;

// 工厂函数
extern "C" BuiltinCommand* createEchoCommand() {
    return &echoCommand;
}
