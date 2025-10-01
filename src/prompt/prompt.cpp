#include "prompt/prompt.h"

#include "utils/colors.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <pwd.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

std::string PromptGenerator::generate(const PromptContext& context) const {
    std::stringstream prompt;

    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    struct passwd* pw = getpwuid(getuid());
    std::string username = pw ? pw->pw_name : "user";
    bool isRoot = (getuid() == 0);

    prompt << (isRoot ? Color::BRIGHT_RED : Color::BRIGHT_CYAN) << Color::BOLD
           << username << Color::RESET
           << Color::BRIGHT_WHITE << "@" << Color::RESET
           << Color::BRIGHT_GREEN << Color::BOLD << hostname << Color::RESET;

    prompt << " " << Color::BRIGHT_BLUE << Color::BOLD
           << getDisplayPath(context) << Color::RESET;

    std::string gitBranch = getGitBranch();
    if (!gitBranch.empty()) {
        prompt << " " << Color::BRIGHT_MAGENTA << "(" << gitBranch << ")" << Color::RESET;
        std::string gitStatus = getGitStatus();
        if (!gitStatus.empty()) {
            prompt << " " << gitStatus;
        }
    }

    if (context.lastExitCode != 0) {
        prompt << " " << Color::BRIGHT_RED << "[" << context.lastExitCode << "]" << Color::RESET;
    }

    auto now = std::chrono::system_clock::now();
    auto asTimeT = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&asTimeT);
    char timeStr[32];
    std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &tm);
    prompt << " " << Color::DIM << timeStr << Color::RESET;

    prompt << "\n";
    if (isRoot) {
        prompt << Color::BRIGHT_RED << "# " << Color::RESET;
    } else {
        prompt << Color::BRIGHT_GREEN << "❯ " << Color::RESET;
    }

    return prompt.str();
}

bool PromptGenerator::isGitRepository() {
    struct stat buffer {};
    return (stat(".git", &buffer) == 0) || (getenv("GIT_DIR") != nullptr);
}

std::string PromptGenerator::getGitBranch() {
    if (!isGitRepository()) return "";

    FILE* pipe = popen("git symbolic-ref --short HEAD 2>/dev/null || git describe --tags --exact-match 2>/dev/null || git rev-parse --short HEAD 2>/dev/null", "r");
    if (!pipe) return "";

    char buffer[128];
    std::string result;
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result = buffer;
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
    }
    pclose(pipe);
    return result.length() > 20 ? result.substr(0, 20) + "..." : result;
}

std::string PromptGenerator::getGitStatus() {
    if (!isGitRepository()) return "";

    FILE* pipe = popen("git status --porcelain 2>/dev/null", "r");
    if (!pipe) return "";

    char buffer[1024];
    std::string output;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    pclose(pipe);

    if (output.empty()) {
        return Color::GREEN + "✓" + Color::RESET;
    }

    int modified = 0;
    int added = 0;
    int deleted = 0;
    int untracked = 0;
    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.length() >= 2) {
            char x = line[0];
            char y = line[1];
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

    std::string status;
    if (modified > 0) status += Color::YELLOW + "●" + std::to_string(modified) + Color::RESET;
    if (added > 0) status += Color::GREEN + "+" + std::to_string(added) + Color::RESET;
    if (deleted > 0) status += Color::RED + "-" + std::to_string(deleted) + Color::RESET;
    if (untracked > 0) status += Color::BRIGHT_BLUE + "?" + std::to_string(untracked) + Color::RESET;

    return status.empty() ? Color::GREEN + "✓" + Color::RESET : status;
}

std::string PromptGenerator::getDisplayPath(const PromptContext& context) {
    std::string displayPath = context.currentDirectory;

    if (!context.homeDirectory.empty() && context.currentDirectory.find(context.homeDirectory) == 0) {
        if (context.currentDirectory == context.homeDirectory) {
            displayPath = "~";
        } else {
            displayPath = "~" + context.currentDirectory.substr(context.homeDirectory.length());
        }
    }

    if (displayPath.length() > 40) {
        size_t pos = displayPath.find_last_of('/');
        if (pos != std::string::npos && pos > 3) {
            displayPath = "..." + displayPath.substr(pos);
        }
    }

    return displayPath;
}

