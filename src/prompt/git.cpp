#include "prompt/git.h"

#include "utils/colors.h"

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

// 初始化静态缓存
GitIntegration::Cache GitIntegration::cache = {};

bool GitIntegration::isGitRepository() {
    struct stat buffer {};
    return (stat(".git", &buffer) == 0) || (getenv("GIT_DIR") != nullptr);
}

std::string GitIntegration::getBranch(bool forceRefresh) {
    if (!isGitRepository()) return "";

    // 获取当前工作目录
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == nullptr) {
        return "";
    }
    std::string currentDir(cwd);

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - cache.branchTime
    ).count();

    // 检查缓存是否有效
    if (!forceRefresh &&
        cache.lastWorkingDir == currentDir &&
        !cache.branch.empty() &&
        elapsed < BRANCH_CACHE_SECONDS) {
        return cache.branch;
    }

    // 缓存失效或强制刷新，重新获取
    std::string result = executeCommand(
        "git symbolic-ref --short HEAD 2>/dev/null || "
        "git describe --tags --exact-match 2>/dev/null || "
        "git rev-parse --short HEAD 2>/dev/null"
    );

    // 截断过长的分支名
    result = result.length() > 20 ? result.substr(0, 20) + "..." : result;

    // 更新缓存
    cache.branch = result;
    cache.branchTime = now;
    cache.lastWorkingDir = currentDir;

    return result;
}

std::string GitIntegration::getStatus(bool forceRefresh) {
    if (!isGitRepository()) return "";

    // 获取当前工作目录
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == nullptr) {
        return "";
    }
    std::string currentDir(cwd);

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - cache.statusTime
    ).count();

    // 检查缓存是否有效
    if (!forceRefresh &&
        cache.lastWorkingDir == currentDir &&
        !cache.status.empty() &&
        elapsed < STATUS_CACHE_SECONDS) {
        return cache.status;
    }

    // 缓存失效或强制刷新，重新获取
    std::string output = executeCommand("git status --porcelain 2>/dev/null");

    std::string result;
    if (output.empty()) {
        result = Color::GREEN + "✓" + Color::RESET;
    } else {
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
        if (modified > 0) {
            result += Color::YELLOW + "●" + std::to_string(modified) + Color::RESET;
        }
        if (added > 0) {
            result += Color::GREEN + "+" + std::to_string(added) + Color::RESET;
        }
        if (deleted > 0) {
            result += Color::RED + "-" + std::to_string(deleted) + Color::RESET;
        }
        if (untracked > 0) {
            result += Color::BRIGHT_BLUE + "?" + std::to_string(untracked) + Color::RESET;
        }

        if (result.empty()) {
            result = Color::GREEN + "✓" + Color::RESET;
        }
    }

    // 更新缓存
    cache.status = result;
    cache.statusTime = now;
    cache.lastWorkingDir = currentDir;

    return result;
}

void GitIntegration::clearCache() {
    cache = Cache{};
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
