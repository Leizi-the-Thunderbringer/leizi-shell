#include "builtin.h"
#include "../utils/colors.h"
#include <iostream>
#include <cstdlib>

/**
 * @brief export 命令实现
 */
class ExportCommand : public BuiltinCommand {
public:
    std::string getName() const override {
        return "export";
    }

    std::string getHelp() const override {
        return "export var=value      Export environment variable";
    }

    BuiltinResult execute(const std::vector<std::string>& args, BuiltinContext& context) override {
        BuiltinResult result;

        if (args.size() < 2) {
            // 显示所有导出的变量
            extern char **environ;
            for (char **env = environ; *env != nullptr; env++) {
                std::cout << "export " << *env << std::endl;
            }
            result.exitCode = 0;
        } else {
            for (size_t i = 1; i < args.size(); ++i) {
                std::string assignment = args[i];
                size_t eq = assignment.find('=');
                if (eq != std::string::npos) {
                    std::string name = assignment.substr(0, eq);
                    std::string value = context.expandVariables(assignment.substr(eq + 1));

                    context.variables.setString(name, value);
                    setenv(name.c_str(), value.c_str(), 1);
                } else {
                    // 导出已存在的变量
                    if (const auto* existing = context.variables.get(assignment)) {
                        setenv(assignment.c_str(), existing->toString().c_str(), 1);
                    }
                }
            }
            result.exitCode = 0;
        }

        context.lastExitCode = result.exitCode;
        return result;
    }
};

/**
 * @brief unset 命令实现
 */
class UnsetCommand : public BuiltinCommand {
public:
    std::string getName() const override {
        return "unset";
    }

    std::string getHelp() const override {
        return "unset var             Unset variable";
    }

    BuiltinResult execute(const std::vector<std::string>& args, BuiltinContext& context) override {
        BuiltinResult result;

        for (size_t i = 1; i < args.size(); ++i) {
            context.variables.erase(args[i]);
            unsetenv(args[i].c_str());
        }

        result.exitCode = 0;
        context.lastExitCode = result.exitCode;
        return result;
    }
};

// 全局实例
static ExportCommand exportCommand;
static UnsetCommand unsetCommand;

// 工厂函数
extern "C" BuiltinCommand* createExportCommand() {
    return &exportCommand;
}

extern "C" BuiltinCommand* createUnsetCommand() {
    return &unsetCommand;
}
