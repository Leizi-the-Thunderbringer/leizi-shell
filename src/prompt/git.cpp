#include "prompt/git.h"

#include "utils/colors.h"

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <sys/stat.h>

bool GitIntegration::isGitRepository() {
    struct stat buffer {};
    return (stat(".git", &buffer) == 0) || (getenv("GIT_DIR") != nullptr);
}

std::string GitIntegration::getBranch() {
    if (!isGitRepository()) return "";

    std::string result = executeCommand(
        "git symbolic-ref --short HEAD 2>/dev/null || "
        "git describe --tags --exact-match 2>/dev/null || "
        "git rev-parse --short HEAD 2>/dev/null"
    );

    // 截断过长的分支名
    return result.length() > 20 ? result.substr(0, 20) + "..." : result;
}

std::string GitIntegration::getStatus() {
    if (!isGitRepository()) return "";

    std::string output = executeCommand("git status --porcelain 2>/dev/null");

    if (output.empty()) {
        return Color::GREEN + "✓" + Color::RESET;
    }

    // 统计各类文件变更
    int modified = 0;
    int added = 0;
    int deleted = 0;
    int untracked = 0;

    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.length() >= 2) {
            char x = line[0];  // 暂存区状态
            char y = line[1];  // 工作区状态

            if (x == '?' && y == '?') {
                ++untracked;
            } else if (x == 'A' || y == 'A') {
                ++added;
            } else if (x == 'D' || y == 'D') {
                ++deleted;
            } else if (x == 'M' || y == 'M') {
                ++modified;
            }
        }
    }

    // 构建状态字符串
    std::string status;
    if (modified > 0) {
        status += Color::YELLOW + "●" + std::to_string(modified) + Color::RESET;
    }
    if (added > 0) {
        status += Color::GREEN + "+" + std::to_string(added) + Color::RESET;
    }
    if (deleted > 0) {
        status += Color::RED + "-" + std::to_string(deleted) + Color::RESET;
    }
    if (untracked > 0) {
        status += Color::BRIGHT_BLUE + "?" + std::to_string(untracked) + Color::RESET;
    }

    return status.empty() ? Color::GREEN + "✓" + Color::RESET : status;
}

std::string GitIntegration::executeCommand(const std::string& command) {
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "";

    char buffer[1024];
    std::string result;

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);

    // 去除尾部换行符
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }

    return result;
}
