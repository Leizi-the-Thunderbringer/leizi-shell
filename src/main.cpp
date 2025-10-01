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
#include <optional>
#include <signal.h>
#include <fcntl.h>

// 版本信息
#define LEIZI_VERSION_MAJOR 1
#define LEIZI_VERSION_MINOR 1
#define LEIZI_VERSION_PATCH 1
#define LEIZI_VERSION_STRING "1.1.1"

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

#include "utils/colors.h"
#include "utils/variables.h"

// 全局变量用于信号处理
static bool g_interrupted = false;
static pid_t g_foregroundPid = -1;

static void signalHandler(int signal) {
    if (signal == SIGINT) {
        g_interrupted = true;
        std::cout << "\n";
    } else if (signal == SIGTSTP) {
        // Ctrl+Z - 暂停前台进程
        if (g_foregroundPid > 0) {
            kill(g_foregroundPid, SIGTSTP);
        }
    }
}

// 作业状态枚举
enum class JobStatus {
    RUNNING,    // 正在运行
    STOPPED,    // 已停止（Ctrl+Z）
    DONE        // 已完成
};

// 作业信息结构
struct Job {
    int jobId;                      // 作业ID
    pid_t pid;                       // 进程ID
    std::string command;             // 命令字符串
    JobStatus status;                // 作业状态
    bool background;                 // 是否后台运行
    std::chrono::system_clock::time_point startTime;  // 启动时间

    Job(int id, pid_t p, const std::string& cmd, bool bg = false)
        : jobId(id), pid(p), command(cmd),
          status(JobStatus::RUNNING), background(bg),
          startTime(std::chrono::system_clock::now()) {}
};

class LeiziShell {
private:
    VariableManager variables;
    std::vector<std::string> commandHistory;
    std::string currentDirectory;
    std::string homeDirectory;
    int lastExitCode = 0;
    bool exitRequested = false;
    std::string historyFile;

    // 作业控制相关
    std::vector<Job> jobs;           // 作业列表
    int nextJobId = 1;                // 下一个作业ID
    pid_t foregroundPid = -1;        // 前台进程PID

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
            } else if (!inQuotes && !inSingleQuotes) {
                // 处理特殊字符（重定向和管道）
                if (c == '|' || c == '>' || c == '<' || c == '&') {
                    // 先保存当前token
                    if (!current.empty()) {
                        tokens.push_back(current);
                        current.clear();
                    }

                    // 处理特殊符号
                    if (c == '|') {
                        tokens.push_back("|");
                    } else if (c == '>') {
                        // 检查是否是 >>
                        if (i + 1 < input.length() && input[i + 1] == '>') {
                            tokens.push_back(">>");
                            ++i;
                        } else {
                            tokens.push_back(">");
                        }
                    } else if (c == '<') {
                        tokens.push_back("<");
                    } else if (c == '&') {
                        // 检查是否是 &>
                        if (i + 1 < input.length() && input[i + 1] == '>') {
                            tokens.push_back("&>");
                            ++i;
                        } else {
                            current += c;  // & 可能是其他用途
                        }
                    }
                } else if (std::isdigit(c) && i + 1 < input.length() && input[i + 1] == '>') {
                    // 处理 2> 和 2>>
                    if (!current.empty()) {
                        tokens.push_back(current);
                        current.clear();
                    }
                    if (c == '2') {
                        if (i + 2 < input.length() && input[i + 2] == '>') {
                            tokens.push_back("2>>");
                            i += 2;
                        } else {
                            tokens.push_back("2>");
                            ++i;
                        }
                    }
                } else if (std::isspace(c)) {
                    if (!current.empty()) {
                        tokens.push_back(current);
                        current.clear();
                    }
                } else {
                    current += c;
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
        return variables.expand(str, [&](const std::string& varName) -> std::optional<std::string> {
            if (varName == "?") {
                return std::to_string(lastExitCode);
            }
            if (varName == "$") {
                return std::to_string(getpid());
            }
            if (varName == "PWD") {
                return currentDirectory;
            }
            if (varName == "HOME") {
                return homeDirectory;
            }
            if (const char* env = getenv(varName.c_str())) {
                return std::string(env);
            }
            return std::nullopt;
        });
    }

    // ==================== 作业控制相关方法 ====================

    // 更新作业状态
    void updateJobStatus() {
        for (auto& job : jobs) {
            if (job.status == JobStatus::RUNNING || job.status == JobStatus::STOPPED) {
                int status;
                pid_t result = waitpid(job.pid, &status, WNOHANG | WUNTRACED);

                if (result > 0) {
                    if (WIFEXITED(status) || WIFSIGNALED(status)) {
                        job.status = JobStatus::DONE;
                        if (job.background) {
                            std::cout << "[" << job.jobId << "]+ Done\t\t"
                                     << job.command << std::endl;
                        }
                    } else if (WIFSTOPPED(status)) {
                        job.status = JobStatus::STOPPED;
                        if (job.background) {
                            std::cout << "[" << job.jobId << "]+ Stopped\t"
                                     << job.command << std::endl;
                        }
                    }
                }
            }
        }

        // 清理已完成的作业
        jobs.erase(std::remove_if(jobs.begin(), jobs.end(),
            [](const Job& job) { return job.status == JobStatus::DONE; }),
            jobs.end());
    }

    // 列出所有作业
    void listJobs() {
        updateJobStatus();

        if (jobs.empty()) {
            std::cout << "No jobs running" << std::endl;
            return;
        }

        for (const auto& job : jobs) {
            std::string statusStr;
            switch (job.status) {
                case JobStatus::RUNNING:
                    statusStr = "Running";
                    break;
                case JobStatus::STOPPED:
                    statusStr = "Stopped";
                    break;
                case JobStatus::DONE:
                    statusStr = "Done";
                    break;
            }

            std::cout << "[" << job.jobId << "]"
                     << (job.background ? "+" : "-") << "  "
                     << statusStr << "\t\t"
                     << job.command << std::endl;
        }
    }

    // 将作业置于前台
    bool foregroundJob(int jobId) {
        updateJobStatus();

        auto it = std::find_if(jobs.begin(), jobs.end(),
            [jobId](const Job& job) { return job.jobId == jobId; });

        if (it == jobs.end()) {
            std::cerr << "leizi: fg: job " << jobId << " not found" << std::endl;
            return false;
        }

        if (it->status == JobStatus::DONE) {
            std::cerr << "leizi: fg: job has terminated" << std::endl;
            jobs.erase(it);
            return false;
        }

        // 如果作业被停止，恢复它
        if (it->status == JobStatus::STOPPED) {
            if (kill(it->pid, SIGCONT) < 0) {
                perror("leizi: fg");
                return false;
            }
            it->status = JobStatus::RUNNING;
        }

        // 将作业设为前台
        it->background = false;
        foregroundPid = it->pid;

        // 等待作业完成
        int status;
        waitpid(it->pid, &status, WUNTRACED);

        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            jobs.erase(it);
            if (WIFEXITED(status)) {
                lastExitCode = WEXITSTATUS(status);
            } else {
                lastExitCode = 128 + WTERMSIG(status);
            }
        } else if (WIFSTOPPED(status)) {
            it->status = JobStatus::STOPPED;
            std::cout << "[" << it->jobId << "]+ Stopped\t"
                     << it->command << std::endl;
        }

        foregroundPid = -1;
        return true;
    }

    // 将作业置于后台
    bool backgroundJob(int jobId) {
        updateJobStatus();

        auto it = std::find_if(jobs.begin(), jobs.end(),
            [jobId](const Job& job) { return job.jobId == jobId; });

        if (it == jobs.end()) {
            std::cerr << "leizi: bg: job " << jobId << " not found" << std::endl;
            return false;
        }

        if (it->status != JobStatus::STOPPED) {
            std::cerr << "leizi: bg: job already running" << std::endl;
            return false;
        }

        // 恢复作业并置于后台
        if (kill(it->pid, SIGCONT) < 0) {
            perror("leizi: bg");
            return false;
        }

        it->status = JobStatus::RUNNING;
        it->background = true;

        std::cout << "[" << it->jobId << "]+ " << it->command << " &" << std::endl;
        return true;
    }

    // 添加新作业
    int addJob(pid_t pid, const std::string& command, bool background) {
        int jobId = nextJobId++;
        jobs.emplace_back(jobId, pid, command, background);

        if (background) {
            std::cout << "[" << jobId << "] " << pid << std::endl;
        }

        return jobId;
    }

    // ==================== 内建命令处理 ====================

    // 执行内建命令（支持重定向的版本）
    bool executeBuiltinWithRedirection(std::vector<std::string> args) {
        if (args.empty()) return false;

        const std::string& cmd = args[0];

        // 这些命令必须在当前进程中执行
        if (cmd == "cd" || cmd == "export" || cmd == "unset" || cmd == "array" || cmd == "exit") {
            return executeBuiltin(args);
        }

        // 其他内建命令可以在子进程中执行以支持重定向
        if (cmd == "echo" || cmd == "pwd" || cmd == "env" || cmd == "help" || cmd == "version" || cmd == "history") {
            // 解析重定向
            Redirection redir = parseRedirection(args);

            // 如果有重定向，在子进程中执行
            if (redir.type != Redirection::NONE) {
                pid_t pid = fork();
                if (pid == 0) {
                    // 子进程
                    signal(SIGINT, SIG_DFL);
                    applyRedirection(redir);
                    executeBuiltin(args);
                    exit(lastExitCode);
                } else if (pid > 0) {
                    // 父进程
                    int status;
                    waitpid(pid, &status, 0);
                    if (WIFEXITED(status)) {
                        lastExitCode = WEXITSTATUS(status);
                    }
                } else {
                    perror("fork");
                    lastExitCode = 1;
                }
                return true;
            } else {
                // 没有重定向，直接执行
                return executeBuiltin(args);
            }
        }

        return false;
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
                    variables.setString("PWD", currentDirectory);
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

                        variables.setString(name, value);
                        setenv(name.c_str(), value.c_str(), 1);
                    } else {
                        // 导出已存在的变量
                        if (const auto* existing = variables.get(assignment)) {
                            setenv(assignment.c_str(), existing->toString().c_str(), 1);
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

                    variables.setArray(name, arrayValues);
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
                if (const auto* var = variables.get(args[1]); var && var->type == VarType::ARRAY) {
                    std::cout << Color::CYAN << args[1] << Color::RESET << "=(";
                    for (size_t i = 0; i < var->arrayValue.size(); ++i) {
                        if (i > 0) std::cout << " ";
                        std::cout << "\"" << Color::GREEN << var->arrayValue[i]
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
        } else if (cmd == "jobs") {
            // 显示作业列表
            listJobs();
            lastExitCode = 0;
            return true;
        } else if (cmd == "fg") {
            // 将作业置于前台
            int jobId = -1;
            if (args.size() > 1) {
                // 处理 %n 格式或直接的数字
                std::string jobStr = args[1];
                if (jobStr[0] == '%') {
                    jobStr = jobStr.substr(1);
                }
                try {
                    jobId = std::stoi(jobStr);
                } catch (...) {
                    std::cerr << "leizi: fg: invalid job specification" << std::endl;
                    lastExitCode = 1;
                    return true;
                }
            } else {
                // 没有指定作业ID，使用最近的作业
                updateJobStatus();
                if (!jobs.empty()) {
                    jobId = jobs.back().jobId;
                } else {
                    std::cerr << "leizi: fg: no current job" << std::endl;
                    lastExitCode = 1;
                    return true;
                }
            }

            if (foregroundJob(jobId)) {
                lastExitCode = 0;
            } else {
                lastExitCode = 1;
            }
            return true;
        } else if (cmd == "bg") {
            // 将作业置于后台
            int jobId = -1;
            if (args.size() > 1) {
                // 处理 %n 格式或直接的数字
                std::string jobStr = args[1];
                if (jobStr[0] == '%') {
                    jobStr = jobStr.substr(1);
                }
                try {
                    jobId = std::stoi(jobStr);
                } catch (...) {
                    std::cerr << "leizi: bg: invalid job specification" << std::endl;
                    lastExitCode = 1;
                    return true;
                }
            } else {
                // 没有指定作业ID，使用最近的停止作业
                updateJobStatus();
                auto it = std::find_if(jobs.rbegin(), jobs.rend(),
                    [](const Job& job) { return job.status == JobStatus::STOPPED; });
                if (it != jobs.rend()) {
                    jobId = it->jobId;
                } else {
                    std::cerr << "leizi: bg: no stopped jobs" << std::endl;
                    lastExitCode = 1;
                    return true;
                }
            }

            if (backgroundJob(jobId)) {
                lastExitCode = 0;
            } else {
                lastExitCode = 1;
            }
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
        std::cout << "  " << Color::GREEN << "jobs" << Color::RESET << "                 List background jobs\n";
        std::cout << "  " << Color::GREEN << "fg [job]" << Color::RESET << "             Bring job to foreground\n";
        std::cout << "  " << Color::GREEN << "bg [job]" << Color::RESET << "             Resume job in background\n";
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

    // I/O 重定向结构
    struct Redirection {
        enum Type {
            NONE = 0,
            OUTPUT = 1,        // >
            OUTPUT_APPEND = 2, // >>
            INPUT = 3,         // <
            ERROR = 4,         // 2>
            ERROR_APPEND = 5,  // 2>>
            BOTH = 6          // &>
        };

        Type type = NONE;
        std::string filename;
    };

    // 解析重定向
    Redirection parseRedirection(std::vector<std::string>& tokens) {
        Redirection redir;

        for (size_t i = 0; i < tokens.size(); ++i) {
            const std::string& token = tokens[i];

            if (token == ">" && i + 1 < tokens.size()) {
                redir.type = Redirection::OUTPUT;
                redir.filename = tokens[i + 1];
                tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
                break;
            } else if (token == ">>" && i + 1 < tokens.size()) {
                redir.type = Redirection::OUTPUT_APPEND;
                redir.filename = tokens[i + 1];
                tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
                break;
            } else if (token == "<" && i + 1 < tokens.size()) {
                redir.type = Redirection::INPUT;
                redir.filename = tokens[i + 1];
                tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
                break;
            } else if (token == "2>" && i + 1 < tokens.size()) {
                redir.type = Redirection::ERROR;
                redir.filename = tokens[i + 1];
                tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
                break;
            } else if (token == "2>>" && i + 1 < tokens.size()) {
                redir.type = Redirection::ERROR_APPEND;
                redir.filename = tokens[i + 1];
                tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
                break;
            } else if (token == "&>" && i + 1 < tokens.size()) {
                redir.type = Redirection::BOTH;
                redir.filename = tokens[i + 1];
                tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
                break;
            }
        }

        return redir;
    }

    // 应用重定向
    void applyRedirection(const Redirection& redir) {
        if (redir.type == Redirection::NONE) return;

        std::string filename = expandVariables(redir.filename);

        switch (redir.type) {
            case Redirection::OUTPUT: {
                int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                break;
            }
            case Redirection::OUTPUT_APPEND: {
                int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (fd < 0) {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                break;
            }
            case Redirection::INPUT: {
                int fd = open(filename.c_str(), O_RDONLY);
                if (fd < 0) {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
                break;
            }
            case Redirection::ERROR: {
                int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDERR_FILENO);
                close(fd);
                break;
            }
            case Redirection::ERROR_APPEND: {
                int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (fd < 0) {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDERR_FILENO);
                close(fd);
                break;
            }
            case Redirection::BOTH: {
                int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);
                close(fd);
                break;
            }
            default:
                break;
        }
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
            std::vector<std::string> cmd = commands[0];

            // 检查是否后台执行
            bool background = false;
            if (!cmd.empty() && cmd.back() == "&") {
                background = true;
                cmd.pop_back();  // 移除&符号
                if (cmd.empty()) return;  // 只有&符号的情况
            }

            if (!executeBuiltinWithRedirection(cmd)) {
                executeExternal(cmd, background);
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

                // 解析重定向（复制命令以避免修改原始命令）
                std::vector<std::string> cmdCopy = commands[i];
                Redirection redir = parseRedirection(cmdCopy);

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

                // 应用文件重定向（会覆盖管道重定向）
                applyRedirection(redir);

                // 展开变量并执行命令
                std::vector<std::string> expandedArgs;
                for (const auto& arg : cmdCopy) {
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
    void executeExternal(std::vector<std::string> args, bool background = false) {
        if (args.empty()) return;

        // 解析重定向
        Redirection redir = parseRedirection(args);

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
            signal(SIGTSTP, SIG_DFL); // 恢复默认的SIGTSTP处理（Ctrl+Z）

            // 如果是后台进程，忽略SIGINT
            if (background) {
                signal(SIGINT, SIG_IGN);
            }

            // 应用重定向
            applyRedirection(redir);

            execvp(argv[0], argv.data());
            std::cerr << "leizi: " << argv[0] << ": command not found" << std::endl;
            exit(127);
        } else if (pid > 0) {
            // 父进程
            if (background) {
                // 后台执行，添加到作业列表
                std::string cmd = args[0];
                for (size_t i = 1; i < args.size(); ++i) {
                    cmd += " " + args[i];
                }
                addJob(pid, cmd, true);
                lastExitCode = 0;
            } else {
                // 前台执行，等待进程完成
                foregroundPid = pid;
                g_foregroundPid = pid;  // 更新全局前台进程ID
                int status;
                waitpid(pid, &status, WUNTRACED);
                foregroundPid = -1;
                g_foregroundPid = -1;  // 清除全局前台进程ID

                if (WIFEXITED(status)) {
                    lastExitCode = WEXITSTATUS(status);
                } else if (WIFSIGNALED(status)) {
                    lastExitCode = 128 + WTERMSIG(status);
                } else if (WIFSTOPPED(status)) {
                    // 进程被停止（Ctrl+Z）
                    std::string cmd = args[0];
                    for (size_t i = 1; i < args.size(); ++i) {
                        cmd += " " + args[i];
                    }
                    int jobId = addJob(pid, cmd, false);
                    auto it = std::find_if(jobs.begin(), jobs.end(),
                        [jobId](const Job& job) { return job.jobId == jobId; });
                    if (it != jobs.end()) {
                        it->status = JobStatus::STOPPED;
                    }
                    std::cout << "[" << jobId << "]+ Stopped\t" << cmd << std::endl;
                    lastExitCode = 148; // 128 + SIGTSTP
                }
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
        signal(SIGTSTP, signalHandler);  // 添加Ctrl+Z处理

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
        variables.setString("PWD", currentDirectory, true);
        variables.setString("HOME", homeDirectory, true);
        variables.setString("SHELL", "/usr/local/bin/leizi", true);
        variables.setString("LEIZI_VERSION", LEIZI_VERSION_STRING, true);

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
            // 更新后台作业状态
            updateJobStatus();
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

// 不会写啊，召唤夷陵老祖帮我写
// 话说莫玄羽献舍算未定行为吗
// 应该是UB吧
// 你觉得呢
// 我觉得是
// 以及，点睛召将术是UB吗
// 应该是
/*他顺手在一名少年出鞘的佩剑上轻轻一抹，在拇指上拉出了一道伤口，转身给她们（纸人）点上了两对眼睛、四只眼珠，随即，退后一步，微微一笑，道：“媚眼含羞合，丹唇逐笑开。不问善与恶，点睛召将来。”*/
// 这就是UB，因为你没法预知点睛之后会发生什么，Clang/GCC/MSVC也不保证这个正常工作
// 风邪盘，并非普通罗盘。
// 不是用来指东南西北的，而是用来指凶邪妖煞的。
// 相当于是smart pointer的概念
// 你看它会自己转向凶邪妖煞
// 这就是它的智能之处
// 但是对于魏无羡来说是smart pointer
// 因为他是修习鬼道开天辟地第一人，号夷陵老祖。
// 不过人类程序员
// 就nullptr dereference了
// 到时候发生什么可就不可控了
// 这就是UB
/*召阴旗，法宝，黑旗，上有纹饰咒文。
夷陵老祖出品，后被仙门百家效仿。
不同品级的召阴旗有不同的画法和威力。画法有误使用稍有不慎便会酿出大祸。*/
/*插在某个活人身上，将会把一定范围内的阴灵、冤魂、凶尸、邪祟都吸引过去，只攻击这名活人。由于被插旗者仿佛变成了活生生的靶子，所以又称“靶旗”。
也可以插在房子上，但房子里必须有活人，那么攻击范围就会扩大至屋子里的所有人。
因为插旗处附近一定阴气缭绕，仿佛黑风盘旋，也被叫做“黑风旗”。*/
// 这是weak pointer的概念
// 但是一样的，如果半夜公司没人你还搞召阴旗的话就是nullptr dereference
// 这也是UB
// 阴虎符
/*用妖兽腹中的罕见铁精炼铸而成，威力巨大且不认主。
是魏无羡生前所炼法宝里最可怕，也是最令人觊觎的一个。
（虎符铸成不易，毁去亦难，极耗费精力和时间。魏无羡当时处境艰难，便暂且留下作为威慑，只将虎符一分为二，让它只有在合并的时候才能够发挥作用，而且绝不轻易使用。只用两次后就下决心彻底销毁了了虎符的一半。而另一半尚未销毁完毕，就被围剿了。）
（后剩下的一半虎符被薛洋大致拼凑复原，虽不能长久使用，威力也不如原件，但已经能造成十分可怕的后果。）*/
// 这就是shared pointer的概念
// 但是一样的，如果你把它弄丢了，或者被别人抢走了
// 你就nullptr dereference了
// 这也是UB
// 可能会段错误，或者在内核模块中摧毁中断向量表
// 这就更惨了
// 这就是UB
// 以一人元神操控尸傀和恶灵。横笛一支吹彻长夜，纵鬼兵鬼将如千军万马，所向披靡，人挡杀人佛挡杀佛。笛声有如天人之音。这夷陵老祖的并发模型有点厉害
// 不会竞态条件吗
// 竞态条件是指多个进程或线程同时访问共享资源，且至少有一个进程在写入资源时，可能导致数据不一致或不可预期的行为。
// 夷陵老祖的并发模型中，元神作为唯一的控制者，确保了对尸傀和恶灵的操作是有序且一致的。
// 通过元神的集中控制，避免了多个实体同时修改同一资源的情况，从而减少了竞态条件的发生。
// 当然，这种模型在现实中可能会面临一些挑战，比如元神的处理能力和响应速度，但在夷陵老祖的设定中，这种集中控制的方式有效地管理了并发操作，确保了整体的稳定性和一致性。
// 这就是夷陵老祖的并发模型
// 靠，这西门子S7-1200不停的死机
// 要去买个新的了
// 以及HwlloChen大佬的华硕天选4不要突然变成天选姬（不过加了AD域应该不会这样的，除非······夷陵老祖给我AD域控制器附体了）
// 以及我的MacBook Pro M1不要半夜自己malloc出1000000个0的heap然后里面存了个夷陵老祖的元神
// 还有，我的iPhone 14 Pro Max不要突然变成iPhone 14 Pro Max夷陵老祖版
// 以及，我的iPad Pro 12.9不要突然变成iPad Pro 12.9夷陵老祖版
// 以及我的ThinkPad X1 Carbon不要突然变成ThinkPad X1 Carbon夷陵老祖版
