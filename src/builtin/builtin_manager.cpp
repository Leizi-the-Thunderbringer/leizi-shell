#include "builtin_manager.h"
#include <iostream>

// 声明所有命令的工厂函数
extern "C" {
    BuiltinCommand* createCdCommand();
    BuiltinCommand* createEchoCommand();
    BuiltinCommand* createExportCommand();
    BuiltinCommand* createUnsetCommand();
    BuiltinCommand* createArrayCommand();
    BuiltinCommand* createHistoryCommand();
    BuiltinCommand* createPwdCommand();
    BuiltinCommand* createExitCommand();
    BuiltinCommand* createClearCommand();
    BuiltinCommand* createHelpCommand();
    BuiltinCommand* createVersionCommand();
}

BuiltinManager::BuiltinManager() {
    // 注册所有内建命令
    registerCommand(createCdCommand());
    registerCommand(createEchoCommand());
    registerCommand(createExportCommand());
    registerCommand(createUnsetCommand());
    registerCommand(createArrayCommand());
    registerCommand(createHistoryCommand());
    registerCommand(createPwdCommand());
    registerCommand(createExitCommand());
    registerCommand(createClearCommand());
    registerCommand(createHelpCommand());
    registerCommand(createVersionCommand());
}

void BuiltinManager::registerCommand(BuiltinCommand* command) {
    if (command) {
        commands[command->getName()] = command;
    }
}

bool BuiltinManager::isBuiltin(const std::string& name) const {
    return commands.find(name) != commands.end();
}

BuiltinResult BuiltinManager::execute(const std::vector<std::string>& args, BuiltinContext& context) {
    BuiltinResult result;

    if (args.empty()) {
        result.exitCode = 1;
        return result;
    }

    auto it = commands.find(args[0]);
    if (it != commands.end()) {
        return it->second->execute(args, context);
    }

    // 命令不存在
    result.exitCode = 127;
    return result;
}

std::vector<std::string> BuiltinManager::getCommandNames() const {
    std::vector<std::string> names;
    names.reserve(commands.size());
    for (const auto& [name, _] : commands) {
        names.push_back(name);
    }
    return names;
}
