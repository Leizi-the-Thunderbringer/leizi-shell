#include "builtin.h"
#include "../utils/colors.h"
#include <unistd.h>
#include <cstdlib>
#include <iostream>

/**
 * @brief cd 命令实现
 */
class CdCommand : public BuiltinCommand {
public:
    std::string getName() const override {
        return "cd";
    }

    std::string getHelp() const override {
        return "cd [dir]              Change directory";
    }

    BuiltinResult execute(const std::vector<std::string>& args, BuiltinContext& context) override {
        BuiltinResult result;

        std::string path;
        if (args.size() > 1) {
            path = context.expandVariables(args[1]);
        } else {
            path = context.homeDirectory;
        }

        if (chdir(path.c_str()) == 0) {
            char* cwd = getcwd(nullptr, 0);
            if (cwd) {
                context.currentDirectory = cwd;
                context.variables.setString("PWD", context.currentDirectory);
                free(cwd);
            }
            result.exitCode = 0;
        } else {
            perror("leizi");
            result.exitCode = 1;
        }

        context.lastExitCode = result.exitCode;
        return result;
    }
};

// 全局实例
static CdCommand cdCommand;

// 工厂函数
extern "C" BuiltinCommand* createCdCommand() {
    return &cdCommand;
}
