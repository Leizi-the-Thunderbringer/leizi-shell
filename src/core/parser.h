#pragma once

#include <string>
#include <vector>

class CommandParser {
public:
    std::vector<std::string> parseCommand(const std::string& input) const;
    std::vector<std::vector<std::string>> parsePipeline(const std::string& input) const;
};

