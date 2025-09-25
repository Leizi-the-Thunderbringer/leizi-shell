/*
 * Leizi Shell - A modern POSIX-compatible shell with ZSH-style arrays and beautiful prompts
 *
 * Features:
 * - POSIX compatibility with dash-like performance
 * - ZSH-style arrays support
 * - Powerlevel10k-inspired beautiful prompts
 * - Smart auto-completion
 * - Git integration
 * - Cross-platform support (Linux, macOS, BSD)
 *
 * Author: logos
 * License: MIT
 * Repository: https://github.com/Leizi-the-Thunderbringer/leizi-shell
 */

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>
#include <chrono>
#include <fstream>
#include <signal.h>

// 版本信息
#define LEIZI_VERSION_MAJOR 1
#define LEIZI_VERSION_MINOR 0
#define LEIZI_VERSION_PATCH 0
#define LEIZI_VERSION_STRING "1.0.1"

// 检查是否有readline库
#ifdef __has_include
    #if __has_include(<readline/readline.h>)
        #include <readline/readline.h>
        #include <readline/history.h>
        #define HAVE_READLINE 1
    #else
        #define HAVE_READLINE 0
    #endif
#else
    // 尝试包含readline，如果失败则在编译时处理
    #ifdef HAVE_LIBREADLINE
        #include <readline/readline.h>
        #include <readline/history.h>
        #define HAVE_READLINE 1
    #else
        #define HAVE_READLINE 0
    #endif
#endif

// ANSI颜色代码
namespace Color {
    const std::string RESET = "\033[0m";
    const std::string BOLD = "\033[1m";
    const std::string DIM = "\033[2m";
    const std::string ITALIC = "\033[3m";
    const std::string UNDERLINE = "\033[4m";

    // 前景色
    const std::string BLACK = "\033[30m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string WHITE = "\033[37m";

    // 亮色前景
    const std::string BRIGHT_BLACK = "\033[90m";
    const std::string BRIGHT_RED = "\033[91m";
    const std::string BRIGHT_GREEN = "\033[92m";
    const std::string BRIGHT_YELLOW = "\033[93m";
    const std::string BRIGHT_BLUE = "\033[94m";
    const std::string BRIGHT_MAGENTA = "\033[95m";
    const std::string BRIGHT_CYAN = "\033[96m";
    const std::string BRIGHT_WHITE = "\033[97m";

    // 背景色
    const std::string BG_BLACK = "\033[40m";
    const std::string BG_RED = "\033[41m";
    const std::string BG_GREEN = "\033[42m";
    const std::string BG_YELLOW = "\033[43m";
    const std::string BG_BLUE = "\033[44m";
    const std::string BG_MAGENTA = "\033[45m";
    const std::string BG_CYAN = "\033[46m";
    const std::string BG_WHITE = "\033[47m";
}

// Shell变量类型
enum class VarType {
    STRING,
    ARRAY,
    INTEGER,
    READONLY
};

// 变量值结构
struct Variable {
    VarType type;
    std::string stringValue;
    std::vector<std::string> arrayValue;
    int intValue;
    bool isReadonly;

    Variable() : type(VarType::STRING), intValue(0), isReadonly(false) {}
    explicit Variable(const std::string& str, bool readonly = false)
        : type(VarType::STRING), stringValue(str), intValue(0), isReadonly(readonly) {}
    explicit Variable(const std::vector<std::string>& arr, bool readonly = false)
        : type(VarType::ARRAY), arrayValue(arr), intValue(0), isReadonly(readonly) {}
    explicit Variable(int i, bool readonly = false)
        : type(VarType::INTEGER), intValue(i), isReadonly(readonly) {}

    std::string toString() const {
        switch (type) {
            case VarType::STRING: return stringValue;
            case VarType::INTEGER: return std::to_string(intValue);
            case VarType::ARRAY: return arrayValue.empty() ? "" : arrayValue[0];
            default: return "";
        }
    }
};

// 全局变量用于信号处理
static bool g_interrupted = false;
static void signalHandler(int signal) {
    if (signal == SIGINT) {
        g_interrupted = true;
        std::cout << "\n";
    }
}

class LeiziShell {
private:
    std::unordered_map<std::string, Variable> variables;
    std::vector<std::string> commandHistory;
    std::string currentDirectory;
    std::string homeDirectory;
    int lastExitCode = 0;
    bool exitRequested = false;
    std::string historyFile;

    // 简单的输入读取函数（当没有readline时使用）
    std::string simpleReadline(const std::string& prompt) {
        std::cout << prompt;
        std::cout.flush();
        std::string input;
        std::getline(std::cin, input);

        if (g_interrupted) {
            g_interrupted = false;
            return "";
        }

        return input;
    }

    // 加载命令历史
    void loadHistory() {
        historyFile = homeDirectory + "/.leizi_history";
        std::ifstream file(historyFile);
        std::string line;

        while (std::getline(file, line) && commandHistory.size() < 1000) {
            if (!line.empty()) {
                commandHistory.push_back(line);
                #if HAVE_READLINE
                add_history(line.c_str());
                #endif
            }
        }
    }

    // 保存命令历史
    void saveHistory() {
        std::ofstream file(historyFile);
        size_t start = commandHistory.size() > 1000 ? commandHistory.size() - 1000 : 0;

        for (size_t i = start; i < commandHistory.size(); ++i) {
            file << commandHistory[i] << "\n";
        }
    }

    // Git状态检测
    bool isGitRepository() const {
        struct stat buffer;
        return (stat(".git", &buffer) == 0) ||
               (getenv("GIT_DIR") != nullptr);
    }

    std::string getGitBranch() const {
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

    std::string getGitStatus() const {
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

        // 分析git状态
        int modified = 0, added = 0, deleted = 0, untracked = 0;
        std::istringstream iss(output);
        std::string line;

        while (std::getline(iss, line)) {
            if (line.length() >= 2) {
                char x = line[0], y = line[1];
                if (x == '?' && y == '?') untracked++;
                else if (x == 'A' || y == 'A') added++;
                else if (x == 'D' || y == 'D') deleted++;
                else if (x == 'M' || y == 'M') modified++;
            }
        }

        std::string status;
        if (modified > 0) status += Color::YELLOW + "●" + std::to_string(modified) + Color::RESET;
        if (added > 0) status += Color::GREEN + "+" + std::to_string(added) + Color::RESET;
        if (deleted > 0) status += Color::RED + "-" + std::to_string(deleted) + Color::RESET;
        if (untracked > 0) status += Color::BRIGHT_BLUE + "?" + std::to_string(untracked) + Color::RESET;

        return status.empty() ? Color::GREEN + "✓" + Color::RESET : status;
    }

    // 获取当前工作目录的简化显示
    std::string getDisplayPath() const {
        std::string displayPath = currentDirectory;

        // 替换家目录为~
        if (!homeDirectory.empty() && currentDirectory.find(homeDirectory) == 0) {
            if (currentDirectory == homeDirectory) {
                displayPath = "~";
            } else {
                displayPath = "~" + currentDirectory.substr(homeDirectory.length());
            }
        }

        // 如果路径太长，进行截断
        if (displayPath.length() > 40) {
            size_t pos = displayPath.find_last_of('/');
            if (pos != std::string::npos && pos > 3) {
                displayPath = "..." + displayPath.substr(pos);
            }
        }

        return displayPath;
    }

    // 美观的提示符 (Powerlevel10k风格)
    std::string generatePrompt() const {
        std::stringstream prompt;

        // 第一行：用户信息和路径
        char hostname[256];
        gethostname(hostname, sizeof(hostname));
        struct passwd* pw = getpwuid(getuid());
        std::string username = pw ? pw->pw_name : "user";

        // 用户名@主机名
        bool isRoot = (getuid() == 0);
        prompt << (isRoot ? Color::BRIGHT_RED : Color::BRIGHT_CYAN) << Color::BOLD
               << username << Color::RESET
               << Color::BRIGHT_WHITE << "@" << Color::RESET
               << Color::BRIGHT_GREEN << Color::BOLD << hostname << Color::RESET;

        // 当前目录
        prompt << " " << Color::BRIGHT_BLUE << Color::BOLD
               << getDisplayPath() << Color::RESET;

        // Git信息
        std::string gitBranch = getGitBranch();
        if (!gitBranch.empty()) {
            prompt << " " << Color::BRIGHT_MAGENTA << "(" << gitBranch << ")" << Color::RESET;
            std::string gitStatus = getGitStatus();
            if (!gitStatus.empty()) {
                prompt << " " << gitStatus;
            }
        }

        // 退出码指示器
        if (lastExitCode != 0) {
            prompt << " " << Color::BRIGHT_RED << "[" << lastExitCode << "]" << Color::RESET;
        }

        // 时间戳
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        char timeStr[32];
        std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &tm);
        prompt << " " << Color::DIM << timeStr << Color::RESET;

        // 第二行：提示符
        prompt << "\n";
        if (isRoot) {
            prompt << Color::BRIGHT_RED << "# " << Color::RESET;
        } else {
            prompt << Color::BRIGHT_GREEN << "❯ " << Color::RESET;
        }

        return prompt.str();
    }

    // 自动补全功能
    std::vector<std::string> getCompletions(const std::string& input) const {
        std::vector<std::string> completions;

        // 分析输入
        auto tokens = parseCommand(input);
        if (tokens.empty()) return completions;

        std::string lastToken = tokens.back();
        bool isFirstToken = (tokens.size() == 1);

        if (isFirstToken) {
            // 补全命令
            std::vector<std::string> builtins = {
                "cd", "pwd", "exit", "echo", "export", "unset", "history",
                "array", "help", "version", "clear", "jobs", "fg", "bg"
            };

            for (const auto& builtin : builtins) {
                if (lastToken.empty() || builtin.find(lastToken) == 0) {
                    completions.push_back(builtin);
                }
            }

            // 补全PATH中的命令
            const char* pathEnv = getenv("PATH");
            if (pathEnv) {
                std::string pathStr = pathEnv;
                std::stringstream ss(pathStr);
                std::string dir;

                while (std::getline(ss, dir, ':')) {
                    if (dir.empty()) continue;

                    DIR* dirp = opendir(dir.c_str());
                    if (dirp) {
                        struct dirent* entry;
                        while ((entry = readdir(dirp)) != nullptr) {
                            std::string name = entry->d_name;
                            if (name != "." && name != ".." &&
                                (lastToken.empty() || name.find(lastToken) == 0)) {

                                std::string fullPath = dir + "/" + name;
                                struct stat st;
                                if (stat(fullPath.c_str(), &st) == 0 &&
                                    (st.st_mode & S_IXUSR)) {
                                    completions.push_back(name);
                                }
                            }
                        }
                        closedir(dirp);
                    }
                }
            }
        }

        // 文件和目录补全
        std::string dirPath = ".";
        std::string prefix = lastToken;

        size_t lastSlash = lastToken.find_last_of('/');
        if (lastSlash != std::string::npos) {
            dirPath = lastToken.substr(0, lastSlash);
            prefix = lastToken.substr(lastSlash + 1);
            if (dirPath.empty()) dirPath = "/";
        }

        DIR* dir = opendir(dirPath.c_str());
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string filename = entry->d_name;
                if (filename != "." && filename != ".." &&
                    (prefix.empty() || filename.find(prefix) == 0)) {

                    std::string fullName = filename;
                    if (dirPath != ".") {
                        fullName = dirPath + "/" + filename;
                    }

                    // 检查是否为目录
                    struct stat st;
                    std::string checkPath = (dirPath == ".") ? filename : (dirPath + "/" + filename);
                    if (stat(checkPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                        fullName += "/";
                    }

                    completions.push_back(fullName);
                }
            }
            closedir(dir);
        }

        // 排序并去重
        std::sort(completions.begin(), completions.end());
        completions.erase(std::unique(completions.begin(), completions.end()), completions.end());

        return completions;
    }

    // 解析命令行
    std::vector<std::string> parseCommand(const std::string& input) const {
        std::vector<std::string> tokens;
        std::string current;
        bool inQuotes = false;
        bool inSingleQuotes = false;
        bool escaped = false;

        for (size_t i = 0; i < input.length(); ++i) {
            char c = input[i];

            if (escaped) {
                current += c;
                escaped = false;
                continue;
            }

            if (c == '\\' && !inSingleQuotes) {
                escaped = true;
                continue;
            }

            if (!inQuotes && !inSingleQuotes && c == '"') {
                inQuotes = true;
            } else if (!inQuotes && !inSingleQuotes && c == '\'') {
                inSingleQuotes = true;
            } else if (inQuotes && c == '"') {
                inQuotes = false;
            } else if (inSingleQuotes && c == '\'') {
                inSingleQuotes = false;
            } else if (!inQuotes && !inSingleQuotes && std::isspace(c)) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }

        if (!current.empty()) {
            tokens.push_back(current);
        }

        return tokens;
    }

    // 变量展开
    std::string expandVariables(const std::string& str) const {
        std::string result = str;
        size_t pos = 0;

        while ((pos = result.find('$', pos)) != std::string::npos) {
            if (pos + 1 >= result.length()) break;

            size_t start = pos + 1;
            size_t end = start;

            // 支持 ${var} 和 $var 语法
            bool braced = (start < result.length() && result[start] == '{');
            if (braced) {
                start++;
                end = result.find('}', start);
                if (end == std::string::npos) {
                    pos++;
                    continue;
                }
            } else {
                while (end < result.length() &&
                       (std::isalnum(result[end]) || result[end] == '_')) {
                    end++;
                }
            }

            if (end > start) {
                std::string varName = result.substr(start, end - start);
                std::string value;

                // 特殊变量
                if (varName == "?") {
                    value = std::to_string(lastExitCode);
                } else if (varName == "$") {
                    value = std::to_string(getpid());
                } else if (varName == "PWD") {
                    value = currentDirectory;
                } else if (varName == "HOME") {
                    value = homeDirectory;
                } else {
                    // 用户定义的变量
                    auto it = variables.find(varName);
                    if (it != variables.end()) {
                        value = it->second.toString();
                    } else {
                        // 环境变量
                        const char* env = getenv(varName.c_str());
                        if (env) value = env;
                    }
                }

                size_t replaceStart = pos;
                size_t replaceEnd = braced ? end + 1 : end;
                result.replace(replaceStart, replaceEnd - replaceStart, value);
                pos = replaceStart + value.length();
            } else {
                pos++;
            }
        }

        return result;
    }

    // 执行内建命令
    bool executeBuiltin(const std::vector<std::string>& args) {
        if (args.empty()) return false;

        const std::string& cmd = args[0];

        if (cmd == "exit") {
            int code = 0;
            if (args.size() > 1) {
                try {
                    code = std::stoi(args[1]);
                } catch (...) {
                    code = 255;
                }
            }
            exitRequested = true;
            lastExitCode = code;
            return true;
        } else if (cmd == "cd") {
            std::string path;
            if (args.size() > 1) {
                path = expandVariables(args[1]);
            } else {
                path = homeDirectory;
            }

            if (chdir(path.c_str()) == 0) {
                char* cwd = getcwd(nullptr, 0);
                if (cwd) {
                    currentDirectory = cwd;
                    variables["PWD"] = Variable(currentDirectory);
                    free(cwd);
                }
                lastExitCode = 0;
            } else {
                perror("leizi");
                lastExitCode = 1;
            }
            return true;
        } else if (cmd == "pwd") {
            std::cout << currentDirectory << std::endl;
            lastExitCode = 0;
            return true;
        } else if (cmd == "echo") {
            bool newline = true;
            size_t start = 1;

            // 处理 -n 选项
            if (args.size() > 1 && args[1] == "-n") {
                newline = false;
                start = 2;
            }

            for (size_t i = start; i < args.size(); ++i) {
                if (i > start) std::cout << " ";
                std::cout << expandVariables(args[i]);
            }
            if (newline) std::cout << std::endl;
            lastExitCode = 0;
            return true;
        } else if (cmd == "export") {
            if (args.size() < 2) {
                // 显示所有导出的变量
                extern char **environ;
                for (char **env = environ; *env != nullptr; env++) {
                    std::cout << "export " << *env << std::endl;
                }
                lastExitCode = 0;
            } else {
                for (size_t i = 1; i < args.size(); ++i) {
                    std::string assignment = args[i];
                    size_t eq = assignment.find('=');
                    if (eq != std::string::npos) {
                        std::string name = assignment.substr(0, eq);
                        std::string value = expandVariables(assignment.substr(eq + 1));

                        variables[name] = Variable(value);
                        setenv(name.c_str(), value.c_str(), 1);
                    } else {
                        // 导出已存在的变量
                        auto it = variables.find(assignment);
                        if (it != variables.end()) {
                            setenv(assignment.c_str(), it->second.toString().c_str(), 1);
                        }
                    }
                }
                lastExitCode = 0;
            }
            return true;
        } else if (cmd == "unset") {
            for (size_t i = 1; i < args.size(); ++i) {
                variables.erase(args[i]);
                unsetenv(args[i].c_str());
            }
            lastExitCode = 0;
            return true;
        } else if (cmd == "array") {
            if (args.size() < 2) {
                std::cout << "Usage: array name=(val1 val2 ...) or array name" << std::endl;
                lastExitCode = 1;
                return true;
            }

            std::string assignment = args[1];
            size_t eq = assignment.find('=');

            if (eq != std::string::npos) {
                // 创建数组
                std::string name = assignment.substr(0, eq);
                std::string values = assignment.substr(eq + 1);

                if (values.front() == '(' && values.back() == ')') {
                    values = values.substr(1, values.length() - 2);
                    std::vector<std::string> arrayValues = parseCommand(values);

                    for (auto& val : arrayValues) {
                        val = expandVariables(val);
                    }

                    variables[name] = Variable(arrayValues);
                    lastExitCode = 0;

                    std::cout << "Array " << Color::CYAN << name << Color::RESET
                              << " created with " << Color::YELLOW << arrayValues.size()
                              << Color::RESET << " elements" << std::endl;
                } else {
                    std::cout << "Error: Array syntax should be name=(val1 val2 ...)" << std::endl;
                    lastExitCode = 1;
                }
            } else {
                // 显示数组内容
                auto it = variables.find(args[1]);
                if (it != variables.end() && it->second.type == VarType::ARRAY) {
                    std::cout << Color::CYAN << args[1] << Color::RESET << "=(";
                    for (size_t i = 0; i < it->second.arrayValue.size(); ++i) {
                        if (i > 0) std::cout << " ";
                        std::cout << "\"" << Color::GREEN << it->second.arrayValue[i]
                                  << Color::RESET << "\"";
                    }
                    std::cout << ")" << std::endl;
                    lastExitCode = 0;
                } else {
                    std::cout << "Array " << Color::RED << args[1]
                              << Color::RESET << " not found" << std::endl;
                    lastExitCode = 1;
                }
            }
            return true;
        } else if (cmd == "history") {
            size_t count = 20; // 默认显示最近20条
            if (args.size() > 1) {
                try {
                    count = std::stoul(args[1]);
                } catch (...) {
                    count = 20;
                }
            }

            size_t start = commandHistory.size() > count ?
                          commandHistory.size() - count : 0;

            for (size_t i = start; i < commandHistory.size(); ++i) {
                std::cout << Color::DIM << std::setw(4) << (i + 1)
                          << Color::RESET << " " << commandHistory[i] << std::endl;
            }
            lastExitCode = 0;
            return true;
        } else if (cmd == "clear") {
            std::cout << "\033[2J\033[H";
            lastExitCode = 0;
            return true;
        } else if (cmd == "help") {
            showHelp();
            return true;
        } else if (cmd == "version") {
            showVersion();
            return true;
        }

        return false;
    }

    // 显示帮助信息
    void showHelp() {
        std::cout << Color::BOLD << Color::CYAN << "Leizi Shell " << LEIZI_VERSION_STRING
                  << Color::RESET << " - A modern POSIX-compatible shell\n\n";

        std::cout << Color::BOLD << "Built-in Commands:" << Color::RESET << "\n";
        std::cout << "  " << Color::GREEN << "cd [dir]" << Color::RESET << "              Change directory\n";
        std::cout << "  " << Color::GREEN << "pwd" << Color::RESET << "                  Print working directory\n";
        std::cout << "  " << Color::GREEN << "echo [-n] text" << Color::RESET << "       Print text\n";
        std::cout << "  " << Color::GREEN << "export var=value" << Color::RESET << "     Export environment variable\n";
        std::cout << "  " << Color::GREEN << "unset var" << Color::RESET << "            Unset variable\n";
        std::cout << "  " << Color::GREEN << "array name=(v1 v2)" << Color::RESET << "   Create/display ZSH-style array\n";
        std::cout << "  " << Color::GREEN << "history [n]" << Color::RESET << "          Show command history\n";
        std::cout << "  " << Color::GREEN << "clear" << Color::RESET << "                Clear screen\n";
        std::cout << "  " << Color::GREEN << "help" << Color::RESET << "                 Show this help\n";
        std::cout << "  " << Color::GREEN << "version" << Color::RESET << "              Show version info\n";
        std::cout << "  " << Color::GREEN << "exit [code]" << Color::RESET << "          Exit shell\n\n";

        std::cout << Color::BOLD << "Features:" << Color::RESET << "\n";
        std::cout << "  • Beautiful Powerlevel10k-inspired prompts\n";
        std::cout << "  • Git integration with branch and status display\n";
        std::cout << "  • ZSH-style array support\n";
        std::cout << "  • Smart tab completion\n";
        std::cout << "  • POSIX compatibility\n";
        std::cout << "  • Variable expansion ($var, ${var})\n";
        std::cout << "  • Command history with persistent storage\n\n";

        std::cout << Color::BOLD << "Variable Expansion:" << Color::RESET << "\n";
        std::cout << "  $var or ${var}       Variable expansion\n";
        std::cout << "  $?                   Last exit code\n";
        std::cout << "  $                   Process ID\n";
        std::cout << "  $PWD                 Current directory\n";
        std::cout << "  $HOME                Home directory\n\n";

        lastExitCode = 0;
    }

    // 显示版本信息
    void showVersion() {
        std::cout << Color::BOLD << Color::CYAN << "Leizi Shell " << LEIZI_VERSION_STRING
                  << Color::RESET << "\n";
        std::cout << "Built with C++20\n";
        std::cout << "Features: POSIX compatibility, ZSH arrays, beautiful prompts\n";
        #if HAVE_READLINE
        std::cout << "Readline support: " << Color::GREEN << "enabled" << Color::RESET << "\n";
        #else
        std::cout << "Readline support: " << Color::YELLOW << "disabled" << Color::RESET << "\n";
        #endif
        std::cout << "Git integration: " << Color::GREEN << "enabled" << Color::RESET << "\n";
        std::cout << "Repository: https://github.com/Leizi-the-Thunderbringer/leizi-shell\n";
        lastExitCode = 0;
    }

    // 解析管道命令
    std::vector<std::vector<std::string>> parsePipeline(const std::string& input) {
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

    // 执行管道命令
    void executePipeline(const std::vector<std::vector<std::string>>& commands) {
        if (commands.empty()) return;

        // 如果只有一个命令，直接执行
        if (commands.size() == 1) {
            if (!executeBuiltin(commands[0])) {
                executeExternal(commands[0]);
            }
            return;
        }

        // 创建管道
        std::vector<std::pair<int, int>> pipes(commands.size() - 1);
        for (size_t i = 0; i < pipes.size(); ++i) {
            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("pipe");
                lastExitCode = 1;
                return;
            }
            pipes[i] = {pipefd[0], pipefd[1]};
        }

        // 执行每个命令
        std::vector<pid_t> pids;
        for (size_t i = 0; i < commands.size(); ++i) {
            pid_t pid = fork();

            if (pid == 0) {
                // 子进程
                signal(SIGINT, SIG_DFL);  // 恢复默认的SIGINT处理

                // 设置输入重定向
                if (i > 0) {
                    dup2(pipes[i - 1].first, STDIN_FILENO);
                }

                // 设置输出重定向
                if (i < commands.size() - 1) {
                    dup2(pipes[i].second, STDOUT_FILENO);
                }

                // 关闭所有管道文件描述符
                for (size_t j = 0; j < pipes.size(); ++j) {
                    close(pipes[j].first);
                    close(pipes[j].second);
                }

                // 展开变量并执行命令
                std::vector<std::string> expandedArgs;
                for (const auto& arg : commands[i]) {
                    expandedArgs.push_back(expandVariables(arg));
                }

                std::vector<char*> argv;
                for (const auto& arg : expandedArgs) {
                    argv.push_back(const_cast<char*>(arg.c_str()));
                }
                argv.push_back(nullptr);

                // 检查是否是内建命令
                if (expandedArgs[0] == "cd" || expandedArgs[0] == "export" ||
                    expandedArgs[0] == "unset" || expandedArgs[0] == "array") {
                    // 内建命令不能在管道中使用
                    std::cerr << "leizi: " << expandedArgs[0]
                              << ": builtin command cannot be used in pipeline\n";
                    exit(1);
                }

                execvp(argv[0], argv.data());
                std::cerr << "leizi: " << argv[0] << ": command not found" << std::endl;
                exit(127);

            } else if (pid > 0) {
                pids.push_back(pid);
            } else {
                perror("fork");
                lastExitCode = 1;

                // 关闭所有管道
                for (size_t j = 0; j < pipes.size(); ++j) {
                    close(pipes[j].first);
                    close(pipes[j].second);
                }
                return;
            }
        }

        // 父进程关闭所有管道
        for (size_t i = 0; i < pipes.size(); ++i) {
            close(pipes[i].first);
            close(pipes[i].second);
        }

        // 等待所有子进程完成
        for (pid_t pid : pids) {
            int status;
            waitpid(pid, &status, 0);

            // 记录最后一个命令的退出状态
            if (pid == pids.back()) {
                if (WIFEXITED(status)) {
                    lastExitCode = WEXITSTATUS(status);
                } else if (WIFSIGNALED(status)) {
                    lastExitCode = 128 + WTERMSIG(status);
                }
            }
        }
    }

    // 执行外部命令
    void executeExternal(const std::vector<std::string>& args) {
        if (args.empty()) return;

        // 展开所有参数中的变量
        std::vector<std::string> expandedArgs;
        for (const auto& arg : args) {
            expandedArgs.push_back(expandVariables(arg));
        }

        // 创建参数数组
        std::vector<char*> argv;
        for (const auto& arg : expandedArgs) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        pid_t pid = fork();
        if (pid == 0) {
            // 子进程
            signal(SIGINT, SIG_DFL);  // 恢复默认的SIGINT处理
            execvp(argv[0], argv.data());
            std::cerr << "leizi: " << argv[0] << ": command not found" << std::endl;
            exit(127);
        } else if (pid > 0) {
            // 父进程
            int status;
            waitpid(pid, &status, 0);

            if (WIFEXITED(status)) {
                lastExitCode = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                lastExitCode = 128 + WTERMSIG(status);
            }
        } else {
            perror("leizi: fork");
            lastExitCode = 1;
        }
    }

public:
    LeiziShell() {
        // 设置信号处理
        signal(SIGINT, signalHandler);

        // 初始化当前目录
        char* cwd = getcwd(nullptr, 0);
        if (cwd) {
            currentDirectory = cwd;
            free(cwd);
        }

        // 初始化家目录
        struct passwd* pw = getpwuid(getuid());
        homeDirectory = pw ? pw->pw_name : "/tmp";
        if (const char* home = getenv("HOME")) {
            homeDirectory = home;
        }

        // 设置默认变量
        variables["PWD"] = Variable(currentDirectory, true);
        variables["HOME"] = Variable(homeDirectory, true);
        variables["SHELL"] = Variable("/usr/local/bin/leizi", true);
        variables["LEIZI_VERSION"] = Variable(LEIZI_VERSION_STRING, true);

        // 加载历史记录
        loadHistory();

        #if HAVE_READLINE
        // 初始化readline
        rl_attempted_completion_function = nullptr;
        using_history();
        #endif
    }

    ~LeiziShell() {
        saveHistory();
    }

    void run() {
        // 显示欢迎信息
        std::cout << Color::BOLD << Color::CYAN << "🚀 Welcome to Leizi Shell "
                  << LEIZI_VERSION_STRING << Color::RESET << std::endl;
        std::cout << Color::DIM << "A modern POSIX-compatible shell with ZSH arrays and beautiful prompts"
                  << Color::RESET << std::endl;
        std::cout << Color::DIM << "Type 'help' for more information" << Color::RESET << std::endl << std::endl;

        std::string input;

        while (!exitRequested) {
            #if HAVE_READLINE
            char* line = readline(generatePrompt().c_str());
            if (!line) {
                // EOF (Ctrl+D)
                std::cout << std::endl;
                break;
            }

            input = std::string(line);
            if (!input.empty()) {
                add_history(line);
                commandHistory.push_back(input);

                // 解析和执行命令（支持管道）
                auto pipeline = parsePipeline(input);
                executePipeline(pipeline);
            }
            free(line);
            #else
            // 简单的输入循环（没有readline时）
            input = simpleReadline(generatePrompt());

            if (std::cin.eof() || g_interrupted) {
                if (std::cin.eof()) {
                    std::cout << std::endl;
                }
                break;
            }

            if (!input.empty()) {
                commandHistory.push_back(input);

                // 解析和执行命令（支持管道）
                auto pipeline = parsePipeline(input);
                executePipeline(pipeline);
            }
            #endif

            // 重置中断标志
            g_interrupted = false;
        }

        std::cout << Color::CYAN << "Thanks for using Leizi Shell! 👋" << Color::RESET << std::endl;
    }

    int getExitCode() const {
        return lastExitCode;
    }
};

int main(int argc, char* argv[]) {
    // 处理命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--version" || arg == "-v") {
            std::cout << "Leizi Shell " << LEIZI_VERSION_STRING << std::endl;
            return 0;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: leizi [options]\n";
            std::cout << "Options:\n";
            std::cout << "  -h, --help     Show this help message\n";
            std::cout << "  -v, --version  Show version information\n";
            return 0;
        }
    }

    try {
        LeiziShell shell;
        shell.run();
        return shell.getExitCode();
    } catch (const std::exception& e) {
        std::cerr << "leizi: fatal error: " << e.what() << std::endl;
        return 1;
    }
}