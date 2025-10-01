#include "core/parser.h"

#include <cctype>
#include <string>
#include <vector>

std::vector<std::string> CommandParser::parseCommand(const std::string& input) const {
    std::vector<std::string> tokens;
    std::string current;
    bool inSingleQuote = false;
    bool inDoubleQuote = false;

    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];

        if (inSingleQuote) {
            if (c == '\'') {
                inSingleQuote = false;
            } else {
                current += c;
            }
        } else if (inDoubleQuote) {
            if (c == '"') {
                inDoubleQuote = false;
            } else if (c == '\\' && i + 1 < input.length()) {
                char next = input[i + 1];
                current += next;
                ++i;
            } else {
                current += c;
            }
        } else {
            if (c == '\'') {
                inSingleQuote = true;
            } else if (c == '"') {
                inDoubleQuote = true;
            } else if (c == '|' || c == '>' || c == '<') {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
                if (c == '>') {
                    if (i + 1 < input.length() && input[i + 1] == '>') {
                        tokens.emplace_back(">>");
                        ++i;
                    } else {
                        tokens.emplace_back(">");
                    }
                } else if (c == '|') {
                    tokens.emplace_back("|");
                } else {
                    tokens.emplace_back("<");
                }
            } else if (c == '&') {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
                if (i + 1 < input.length() && input[i + 1] == '>') {
                    tokens.emplace_back("&>");
                    ++i;
                } else {
                    current += c;
                }
            } else if (std::isdigit(static_cast<unsigned char>(c)) && i + 1 < input.length() && input[i + 1] == '>') {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
                if (c == '2' && i + 2 < input.length() && input[i + 2] == '>') {
                    tokens.emplace_back("2>>");
                    i += 2;
                } else {
                    tokens.emplace_back("2>");
                    ++i;
                }
            } else if (std::isspace(static_cast<unsigned char>(c))) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

std::vector<std::vector<std::string>> CommandParser::parsePipeline(const std::string& input) const {
    std::vector<std::vector<std::string>> commands;
    std::vector<std::string> currentCmd;
    auto tokens = parseCommand(input);

    for (const auto& token : tokens) {
        if (token == "|") {
            if (!currentCmd.empty()) {
                commands.push_back(currentCmd);
                currentCmd.clear();
            }
        } else {
            currentCmd.push_back(token);
        }
    }

    if (!currentCmd.empty()) {
        commands.push_back(currentCmd);
    }

    return commands;
}

