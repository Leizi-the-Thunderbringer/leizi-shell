#pragma once

#include <string>

struct PromptContext {
    std::string currentDirectory;
    std::string homeDirectory;
    int lastExitCode = 0;
};

class PromptGenerator {
public:
    std::string generate(const PromptContext& context) const;

private:
    static bool isGitRepository();
    static std::string getGitBranch();
    static std::string getGitStatus();
    static std::string getDisplayPath(const PromptContext& context);
};

