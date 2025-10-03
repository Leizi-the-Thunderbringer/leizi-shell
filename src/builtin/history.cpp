#include "builtin.h"
#include "../utils/colors.h"
#include <iostream>
#include <iomanip>

/**
 * @brief history 命令实现
 */
class HistoryCommand : public BuiltinCommand {
public:
    std::string getName() const override {
        return "history";
    }

    std::string getHelp() const override {
        return "history [n]           Show command history";
    }

    BuiltinResult execute(const std::vector<std::string>& args, BuiltinContext& context) override {
        BuiltinResult result;

        size_t count = 20; // 默认显示最近20条
        if (args.size() > 1) {
            try {
                count = std::stoul(args[1]);
            } catch (...) {
                count = 20;
            }
        }

        size_t start = context.commandHistory.size() > count ?
                      context.commandHistory.size() - count : 0;

        for (size_t i = start; i < context.commandHistory.size(); ++i) {
            std::cout << Color::DIM << std::setw(4) << (i + 1)
                      << Color::RESET << " " << context.commandHistory[i] << std::endl;
        }

        result.exitCode = 0;
        context.lastExitCode = result.exitCode;
        return result;
    }
};

// 全局实例
static HistoryCommand historyCommand;

// 工厂函数
extern "C" BuiltinCommand* createHistoryCommand() {
    return &historyCommand;
}
