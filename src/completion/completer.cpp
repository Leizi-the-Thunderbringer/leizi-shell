#include "completion/completer.h"
#include "core/parser.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <pwd.h>

// 全局environ声明 (必须在namespace外)
extern char** environ;

namespace leizi {

// ========== CommandCompleter ==========

CommandCompleter::CommandCompleter(const std::vector<std::string>& builtins)
    : builtinCommands(builtins) {}

std::vector<std::string> CommandCompleter::getCompletions(const CompletionContext& ctx) {
    if (!ctx.isFirstToken) {
        return {};  // 只在第一个token时补全命令
    }

    std::vector<std::string> completions;
    const std::string& prefix = ctx.currentToken;

    // 补全内建命令
    for (const auto& cmd : builtinCommands) {
        if (prefix.empty() || cmd.find(prefix) == 0) {
            completions.push_back(cmd);
        }
    }

    // 补全PATH中的命令
    auto pathCmds = getPathCommands(prefix);
    completions.insert(completions.end(), pathCmds.begin(), pathCmds.end());

    return completions;
}

std::vector<std::string> CommandCompleter::getPathCommands(const std::string& prefix) const {
    std::vector<std::string> commands;
    const char* pathEnv = getenv("PATH");
    if (!pathEnv) return commands;

    std::string pathStr = pathEnv;
    std::stringstream ss(pathStr);
    std::string dir;

    while (std::getline(ss, dir, ':')) {
        if (dir.empty()) continue;

        DIR* dirp = opendir(dir.c_str());
        if (!dirp) continue;

        struct dirent* entry;
        while ((entry = readdir(dirp)) != nullptr) {
            std::string name = entry->d_name;
            if (name == "." || name == "..") continue;
            if (!prefix.empty() && name.find(prefix) != 0) continue;

            std::string fullPath = dir + "/" + name;
            struct stat st;
            if (stat(fullPath.c_str(), &st) == 0 && (st.st_mode & S_IXUSR)) {
                commands.push_back(name);
            }
        }
        closedir(dirp);
    }

    return commands;
}

// ========== FileCompleter ==========

std::vector<std::string> FileCompleter::getCompletions(const CompletionContext& ctx) {
    std::vector<std::string> completions;
    std::string input = expandTilde(ctx.currentToken);

    std::string dirPath = ".";
    std::string prefix = input;

    size_t lastSlash = input.find_last_of('/');
    if (lastSlash != std::string::npos) {
        dirPath = input.substr(0, lastSlash);
        prefix = input.substr(lastSlash + 1);
        if (dirPath.empty()) dirPath = "/";
    }

    DIR* dir = opendir(dirPath.c_str());
    if (!dir) return completions;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        if (filename == "." || filename == "..") continue;
        if (!prefix.empty() && filename.find(prefix) != 0) continue;

        std::string fullName = filename;
        if (dirPath != ".") {
            fullName = dirPath + "/" + filename;
        }

        // 检查是否为目录，添加 /
        std::string checkPath = (dirPath == ".") ? filename : (dirPath + "/" + filename);
        struct stat st;
        if (stat(checkPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            fullName += "/";
        }

        completions.push_back(fullName);
    }
    closedir(dir);

    return completions;
}

std::string FileCompleter::expandTilde(const std::string& path) const {
    if (path.empty() || path[0] != '~') {
        return path;
    }

    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) home = pw->pw_dir;
    }

    if (!home) return path;

    if (path.length() == 1 || path[1] == '/') {
        return std::string(home) + path.substr(1);
    }

    return path;  // ~user 格式暂不支持
}

// ========== VariableCompleter ==========

VariableCompleter::VariableCompleter(const VariableManager& vm)
    : variables(vm) {}

std::vector<std::string> VariableCompleter::getCompletions(const CompletionContext& ctx) {
    const std::string& token = ctx.currentToken;

    // 只在token以$开头时补全变量
    if (token.empty() || token[0] != '$') {
        return {};
    }

    std::vector<std::string> completions;
    std::string prefix = token.substr(1);  // 移除$符号

    // 遍历环境变量
    for (char** env = environ; *env != nullptr; ++env) {
        std::string envStr = *env;
        size_t eq = envStr.find('=');
        if (eq != std::string::npos) {
            std::string varName = envStr.substr(0, eq);
            if (prefix.empty() || varName.find(prefix) == 0) {
                completions.push_back("$" + varName);
            }
        }
    }

    // 特殊变量
    std::vector<std::string> specials = {"$?", "$$", "$PWD", "$HOME", "$USER", "$PATH"};
    for (const auto& var : specials) {
        if (token.empty() || var.find(token) == 0) {
            completions.push_back(var);
        }
    }

    return completions;
}

// ========== HistoryCompleter ==========

HistoryCompleter::HistoryCompleter(const std::vector<std::string>& history)
    : commandHistory(history) {}

std::vector<std::string> HistoryCompleter::getCompletions(const CompletionContext& ctx) {
    if (!ctx.isFirstToken) {
        return {};  // 只在第一个token时从历史补全
    }

    std::vector<std::string> completions;
    const std::string& prefix = ctx.currentToken;

    // 从历史记录中查找匹配的命令
    for (auto it = commandHistory.rbegin(); it != commandHistory.rend(); ++it) {
        const std::string& cmd = *it;
        if (cmd.empty()) continue;

        // 提取第一个token (命令名)
        size_t space = cmd.find(' ');
        std::string cmdName = (space != std::string::npos) ? cmd.substr(0, space) : cmd;

        if (prefix.empty() || cmdName.find(prefix) == 0) {
            completions.push_back(cmdName);
        }
    }

    return completions;
}

// ========== SmartCompleter ==========

SmartCompleter::SmartCompleter() = default;

void SmartCompleter::addProvider(std::unique_ptr<CompletionProvider> provider) {
    providers.push_back(std::move(provider));

    // 按优先级排序 (高优先级在前)
    std::sort(providers.begin(), providers.end(),
              [](const auto& a, const auto& b) {
                  return a->priority() > b->priority();
              });
}

std::vector<std::string> SmartCompleter::getCompletions(const std::string& input) {
    CompletionContext ctx = analyzeInput(input);
    std::vector<std::string> allCompletions;

    // 从所有provider收集补全
    for (const auto& provider : providers) {
        auto completions = provider->getCompletions(ctx);
        allCompletions.insert(allCompletions.end(), completions.begin(), completions.end());
    }

    // 排序并去重
    std::sort(allCompletions.begin(), allCompletions.end());
    allCompletions.erase(std::unique(allCompletions.begin(), allCompletions.end()),
                         allCompletions.end());

    return allCompletions;
}

CompletionContext SmartCompleter::analyzeInput(const std::string& input) const {
    CompletionContext ctx;
    ctx.fullInput = input;

    // 使用CommandParser解析
    CommandParser parser;
    ctx.tokens = parser.parseCommand(input);

    if (ctx.tokens.empty()) {
        ctx.currentToken = "";
        ctx.tokenIndex = 0;
        ctx.isFirstToken = true;
    } else {
        ctx.currentToken = ctx.tokens.back();
        ctx.tokenIndex = ctx.tokens.size() - 1;
        ctx.isFirstToken = (ctx.tokens.size() == 1);
    }

    return ctx;
}

} // namespace leizi
