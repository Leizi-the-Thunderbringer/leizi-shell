#ifndef LEIZI_BUILTIN_CONTEXT_H
#define LEIZI_BUILTIN_CONTEXT_H

#include <string>
#include <vector>
#include <functional>
#include "../utils/variables.h"
#include "../core/parser.h"

/**
 * @brief 内建命令执行上下文
 *
 * 提供内建命令访问 shell 状态和功能的接口
 */
struct BuiltinContext {
    // Shell 状态
    VariableManager& variables;
    CommandParser& parser;
    std::vector<std::string>& commandHistory;
    std::string& currentDirectory;
    const std::string& homeDirectory;
    int& lastExitCode;
    bool& exitRequested;
    const std::string& historyFile;

    // 辅助函数
    std::function<std::string(const std::string&)> expandVariables;

    BuiltinContext(
        VariableManager& vars,
        CommandParser& p,
        std::vector<std::string>& hist,
        std::string& currDir,
        const std::string& homeDir,
        int& exitCode,
        bool& exit,
        const std::string& histFile,
        std::function<std::string(const std::string&)> expander
    ) : variables(vars), parser(p), commandHistory(hist),
        currentDirectory(currDir), homeDirectory(homeDir),
        lastExitCode(exitCode), exitRequested(exit),
        historyFile(histFile), expandVariables(expander) {}
};

#endif // LEIZI_BUILTIN_CONTEXT_H