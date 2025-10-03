#include "syntax/highlighter.h"
#include "utils/colors.h"

#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace leizi {

SyntaxHighlighter::SyntaxHighlighter(const std::vector<std::string>& builtinCommands) {
    for (const auto& cmd : builtinCommands) {
        builtinCommands_.insert(cmd);
    }
    cachePathCommands();
}

std::string SyntaxHighlighter::highlight(const std::string& input) {
    if (input.empty()) return input;

    std::string result;
    size_t pos = 0;
    bool isFirstToken = true;

    while (pos < input.length()) {
        // 跳过空白
        while (pos < input.length() && std::isspace(input[pos])) {
            result += input[pos++];
        }

        if (pos >= input.length()) break;

        // 检查字符串（单引号或双引号）
        if (input[pos] == '\'' || input[pos] == '"') {
            char quote = input[pos];
            size_t end = findStringEnd(input, pos + 1, quote);

            // 字符串用黄色
            result += Color::YELLOW;
            result += input.substr(pos, end - pos + 1);
            result += Color::RESET;

            pos = end + 1;
            isFirstToken = false;
            continue;
        }

        // 检查变量 ($VAR)
        if (input[pos] == '$') {
            size_t end = pos + 1;

            // ${VAR} 形式
            if (end < input.length() && input[end] == '{') {
                end++;
                while (end < input.length() && input[end] != '}') {
                    end++;
                }
                if (end < input.length()) end++; // 包含 }
            }
            // $VAR 形式
            else {
                while (end < input.length() &&
                       (std::isalnum(input[end]) || input[end] == '_')) {
                    end++;
                }
            }

            // 变量用蓝色
            result += Color::BLUE;
            result += input.substr(pos, end - pos);
            result += Color::RESET;

            pos = end;
            isFirstToken = false;
            continue;
        }

        // 检查操作符和重定向
        if (isOperator(input[pos])) {
            size_t end = pos;

            // 处理多字符操作符 (>>, 2>, &>, 等)
            if (input[pos] == '>' || input[pos] == '<' || input[pos] == '&') {
                end++;
                if (end < input.length() && (input[end] == '>' || input[end] == '&')) {
                    end++;
                }
            }
            // 处理 2> 和 2>>
            else if (std::isdigit(input[pos]) && pos + 1 < input.length() &&
                     input[pos + 1] == '>') {
                end += 2;
                if (end < input.length() && input[end] == '>') {
                    end++;
                }
            }
            // 单字符操作符
            else {
                end++;
            }

            // 操作符用紫色
            result += Color::MAGENTA;
            result += input.substr(pos, end - pos);
            result += Color::RESET;

            pos = end;
            continue;
        }

        // 普通单词（命令或参数）
        size_t end = pos;
        while (end < input.length() &&
               !std::isspace(input[end]) &&
               !isOperator(input[end]) &&
               input[end] != '\'' &&
               input[end] != '"' &&
               input[end] != '$') {
            end++;
        }

        std::string token = input.substr(pos, end - pos);

        // 如果是第一个token，检查是否为有效命令
        if (isFirstToken) {
            if (isValidCommand(token)) {
                // 有效命令用绿色
                result += Color::GREEN;
            } else {
                // 无效命令用红色
                result += Color::RED;
            }
            result += token;
            result += Color::RESET;
            isFirstToken = false;
        } else {
            // 参数用默认颜色
            result += token;
        }

        pos = end;
    }

    return result;
}

bool SyntaxHighlighter::isValidCommand(const std::string& command) {
    // 检查内建命令
    if (builtinCommands_.find(command) != builtinCommands_.end()) {
        return true;
    }

    // 检查PATH中的命令
    if (pathCommands_.find(command) != pathCommands_.end()) {
        return true;
    }

    // 检查相对/绝对路径的可执行文件
    if (command.find('/') != std::string::npos) {
        struct stat st;
        if (stat(command.c_str(), &st) == 0 &&
            (st.st_mode & S_IXUSR)) {
            return true;
        }
    }

    return false;
}

void SyntaxHighlighter::refreshPathCache() {
    cachePathCommands();
}

void SyntaxHighlighter::cachePathCommands() {
    pathCommands_.clear();

    const char* pathEnv = std::getenv("PATH");
    if (!pathEnv) return;

    std::string pathStr(pathEnv);
    std::istringstream iss(pathStr);
    std::string dir;

    while (std::getline(iss, dir, ':')) {
        if (dir.empty()) continue;

        try {
            if (!fs::exists(dir) || !fs::is_directory(dir)) {
                continue;
            }

            for (const auto& entry : fs::directory_iterator(dir)) {
                if (entry.is_regular_file() || entry.is_symlink()) {
                    struct stat st;
                    if (stat(entry.path().c_str(), &st) == 0 &&
                        (st.st_mode & S_IXUSR)) {
                        pathCommands_.insert(entry.path().filename().string());
                    }
                }
            }
        } catch (const fs::filesystem_error&) {
            // 忽略无法访问的目录
            continue;
        }
    }
}

size_t SyntaxHighlighter::findStringEnd(const std::string& input, size_t pos, char quote) {
    while (pos < input.length()) {
        if (input[pos] == quote) {
            return pos;
        }
        // 处理转义字符
        if (input[pos] == '\\' && pos + 1 < input.length()) {
            pos += 2;
        } else {
            pos++;
        }
    }
    // 未找到结束引号，返回字符串末尾
    return input.length() - 1;
}

bool SyntaxHighlighter::isOperator(char c) {
    return c == '|' || c == '>' || c == '<' || c == '&' || c == ';';
}

} // namespace leizi
