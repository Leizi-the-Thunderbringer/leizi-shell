#include "prompt/prompt.h"

#include "prompt/git.h"
#include "utils/colors.h"

#include <chrono>
#include <ctime>
#include <pwd.h>
#include <sstream>
#include <string>
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

    std::string gitBranch = GitIntegration::getBranch();
    if (!gitBranch.empty()) {
        prompt << " " << Color::BRIGHT_MAGENTA << "(" << gitBranch << ")" << Color::RESET;
        std::string gitStatus = GitIntegration::getStatus();
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

