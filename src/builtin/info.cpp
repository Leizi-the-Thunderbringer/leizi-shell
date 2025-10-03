#include "builtin.h"
#include "../utils/colors.h"
#include <iostream>

// 版本信息
#define LEIZI_VERSION_STRING "1.1.1"

// 检查是否有readline库
#ifdef __has_include
    #if __has_include(<readline/readline.h>)
        #define HAVE_READLINE 1
    #else
        #define HAVE_READLINE 0
    #endif
#else
    #ifdef HAVE_LIBREADLINE
        #define HAVE_READLINE 1
    #else
        #define HAVE_READLINE 0
    #endif
#endif

/**
 * @brief help 命令实现
 */
class HelpCommand : public BuiltinCommand {
public:
    std::string getName() const override {
        return "help";
    }

    std::string getHelp() const override {
        return "help                  Show this help";
    }

    BuiltinResult execute(const std::vector<std::string>& args, BuiltinContext& context) override {
        BuiltinResult result;

        std::cout << Color::BOLD << Color::CYAN << "Leizi Shell " << LEIZI_VERSION_STRING
                  << Color::RESET << " - A modern POSIX-compatible shell\n\n";

        std::cout << Color::BOLD << "Built-in Commands:" << Color::RESET << "\n";
        std::cout << "  " << Color::GREEN << "cd [dir]" << Color::RESET << "              Change directory\n";
        std::cout << "  " << Color::GREEN << "pwd" << Color::RESET << "                  Print working directory\n";
        std::cout << "  " << Color::GREEN << "echo [-n] text" << Color::RESET << "       Print text\n";
        std::cout << "  " << Color::GREEN << "export var=value" << Color::RESET << "     Export environment variable\n";
        std::cout << "  " << Color::GREEN << "unset var" << Color::RESET << "            Unset variable\n";
        std::cout << "  " << Color::GREEN << "array name=(v1 v2)" << Color::RESET << "   Create/display ZSH-style array\n";
        std::cout << "  " << Color::GREEN << "history [n]" << Color::RESET << "          Show command history\n";
        std::cout << "  " << Color::GREEN << "jobs" << Color::RESET << "                 List background jobs\n";
        std::cout << "  " << Color::GREEN << "fg [job]" << Color::RESET << "             Bring job to foreground\n";
        std::cout << "  " << Color::GREEN << "bg [job]" << Color::RESET << "             Resume job in background\n";
        std::cout << "  " << Color::GREEN << "clear" << Color::RESET << "                Clear screen\n";
        std::cout << "  " << Color::GREEN << "help" << Color::RESET << "                 Show this help\n";
        std::cout << "  " << Color::GREEN << "version" << Color::RESET << "              Show version info\n";
        std::cout << "  " << Color::GREEN << "exit [code]" << Color::RESET << "          Exit shell\n\n";

        std::cout << Color::BOLD << "Features:" << Color::RESET << "\n";
        std::cout << "  • Beautiful Powerlevel10k-inspired prompts\n";
        std::cout << "  • Git integration with branch and status display\n";
        std::cout << "  • ZSH-style array support\n";
        std::cout << "  • Smart tab completion\n";
        std::cout << "  • POSIX compatibility\n";
        std::cout << "  • Variable expansion ($var, ${var})\n";
        std::cout << "  • Command history with persistent storage\n";
        std::cout << "  • Job control (background execution, fg/bg)\n\n";

        std::cout << Color::BOLD << "Variable Expansion:" << Color::RESET << "\n";
        std::cout << "  $var or ${var}       Variable expansion\n";
        std::cout << "  $?                   Last exit code\n";
        std::cout << "  $$                   Process ID\n";
        std::cout << "  $PWD                 Current directory\n";
        std::cout << "  $HOME                Home directory\n\n";

        result.exitCode = 0;
        context.lastExitCode = result.exitCode;
        return result;
    }
};

/**
 * @brief version 命令实现
 */
class VersionCommand : public BuiltinCommand {
public:
    std::string getName() const override {
        return "version";
    }

    std::string getHelp() const override {
        return "version               Show version info";
    }

    BuiltinResult execute(const std::vector<std::string>& args, BuiltinContext& context) override {
        BuiltinResult result;

        std::cout << Color::BOLD << Color::CYAN << "Leizi Shell " << LEIZI_VERSION_STRING
                  << Color::RESET << "\n";
        std::cout << "Built with C++20\n";
        std::cout << "Features: POSIX compatibility, ZSH arrays, beautiful prompts\n";
        #if HAVE_READLINE
        std::cout << "Readline support: " << Color::GREEN << "enabled" << Color::RESET << "\n";
        #else
        std::cout << "Readline support: " << Color::YELLOW << "disabled" << Color::RESET << "\n";
        #endif
        std::cout << "Git integration: " << Color::GREEN << "enabled" << Color::RESET << "\n";
        std::cout << "Repository: https://github.com/Zixiao-System/leizi-shell\n";

        result.exitCode = 0;
        context.lastExitCode = result.exitCode;
        return result;
    }
};

// 全局实例
static HelpCommand helpCommand;
static VersionCommand versionCommand;

// 工厂函数
extern "C" BuiltinCommand* createHelpCommand() {
    return &helpCommand;
}

extern "C" BuiltinCommand* createVersionCommand() {
    return &versionCommand;
}
