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
    static std::string getDisplayPath(const PromptContext& context);
};

