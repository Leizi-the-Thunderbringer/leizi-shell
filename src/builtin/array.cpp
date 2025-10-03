#include "builtin.h"
#include "../utils/colors.h"
#include <iostream>

/**
 * @brief array 命令实现
 */
class ArrayCommand : public BuiltinCommand {
public:
    std::string getName() const override {
        return "array";
    }

    std::string getHelp() const override {
        return "array name=(v1 v2)    Create/display ZSH-style array";
    }

    BuiltinResult execute(const std::vector<std::string>& args, BuiltinContext& context) override {
        BuiltinResult result;

        if (args.size() < 2) {
            std::cout << "Usage: array name=(val1 val2 ...) or array name" << std::endl;
            result.exitCode = 1;
            context.lastExitCode = result.exitCode;
            return result;
        }

        std::string assignment = args[1];
        size_t eq = assignment.find('=');

        if (eq != std::string::npos) {
            // 创建数组
            std::string name = assignment.substr(0, eq);
            std::string values = assignment.substr(eq + 1);

            if (values.front() == '(' && values.back() == ')') {
                values = values.substr(1, values.length() - 2);
                std::vector<std::string> arrayValues = context.parser.parseCommand(values);

                for (auto& val : arrayValues) {
                    val = context.expandVariables(val);
                }

                context.variables.setArray(name, arrayValues);
                result.exitCode = 0;

                std::cout << "Array " << Color::CYAN << name << Color::RESET
                          << " created with " << Color::YELLOW << arrayValues.size()
                          << Color::RESET << " elements" << std::endl;
            } else {
                std::cout << "Error: Array syntax should be name=(val1 val2 ...)" << std::endl;
                result.exitCode = 1;
            }
        } else {
            // 显示数组内容
            if (const auto* var = context.variables.get(args[1]); var && var->type == VarType::ARRAY) {
                std::cout << Color::CYAN << args[1] << Color::RESET << "=(";
                for (size_t i = 0; i < var->arrayValue.size(); ++i) {
                    if (i > 0) std::cout << " ";
                    std::cout << "\"" << Color::GREEN << var->arrayValue[i]
                              << Color::RESET << "\"";
                }
                std::cout << ")" << std::endl;
                result.exitCode = 0;
            } else {
                std::cout << "Array " << Color::RED << args[1]
                          << Color::RESET << " not found" << std::endl;
                result.exitCode = 1;
            }
        }

        context.lastExitCode = result.exitCode;
        return result;
    }
};

// 全局实例
static ArrayCommand arrayCommand;

// 工厂函数
extern "C" BuiltinCommand* createArrayCommand() {
    return &arrayCommand;
}
