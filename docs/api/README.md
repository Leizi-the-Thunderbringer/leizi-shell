# Leizi Shell API 文档

## 📚 核心模块

### 1. 核心系统 (src/core/)

#### CommandParser

命令解析器，负责解析用户输入。

```cpp
class CommandParser {
public:
    ParsedCommand parse(const std::string& input);
    bool isPipelineCommand(const std::string& input);
};
```

**功能**:
- 命令分词
- 管道识别
- 重定向解析
- 后台作业检测 (`&`)

#### JobControl

作业控制管理器。

```cpp
class JobControl {
public:
    int addJob(pid_t pid, const std::string& command, bool background);
    void listJobs();
    bool foregroundJob(int jobId);
    bool backgroundJob(int jobId);
};
```

### 2. 内建命令 (src/builtin/)

#### BuiltinCommand 基类

```cpp
class BuiltinCommand {
public:
    virtual ~BuiltinCommand() = default;
    virtual BuiltinResult execute(const std::vector<std::string>& args,
                                   BuiltinContext& context) = 0;
    virtual std::string getName() const = 0;
    virtual std::string getHelp() const = 0;
};
```

#### BuiltinManager

```cpp
class BuiltinManager {
public:
    void registerCommand(BuiltinCommand* command);
    bool isBuiltin(const std::string& name) const;
    BuiltinResult execute(const std::vector<std::string>& args,
                          BuiltinContext& context);
    std::vector<std::string> getCommandNames() const;
};
```

**内建命令列表**:
- CdCommand
- EchoCommand
- ExportCommand
- ArrayCommand
- HistoryCommand
- JobsCommand (内置于主程序)
- HighlightCommand

### 3. 提示符系统 (src/prompt/)

#### PromptGenerator

```cpp
class PromptGenerator {
public:
    std::string generate(const PromptContext& context);
};

struct PromptContext {
    std::string currentDirectory;
    std::string homeDirectory;
    int lastExitCode;
};
```

#### GitIntegration

```cpp
class GitIntegration {
public:
    static bool isGitRepository();
    static std::string getBranch(bool forceRefresh = false);
    static std::string getStatus(bool forceRefresh = false);
    static void clearCache();
};
```

**缓存策略**:
- 分支名缓存: 10 秒
- 状态缓存: 2 秒
- 基于工作目录的智能失效

### 4. 变量系统 (src/utils/)

#### VariableManager

```cpp
class VariableManager {
public:
    void setString(const std::string& name, const std::string& value,
                   bool exported = false);
    void setArray(const std::string& name,
                  const std::vector<std::string>& values);
    Variable* get(const std::string& name);
    std::string expand(const std::string& str,
                       std::function<std::optional<std::string>(const std::string&)>
                       specialVars);
};
```

**变量类型**:
```cpp
enum class VarType { STRING, ARRAY };

struct Variable {
    VarType type;
    std::variant<std::string, std::vector<std::string>> value;
    bool exported;

    std::string toString() const;
    std::vector<std::string> toArray() const;
};
```

### 5. 补全系统 (src/completion/)

#### SmartCompleter

```cpp
class SmartCompleter {
public:
    void addProvider(std::unique_ptr<CompletionProvider> provider);
    std::vector<std::string> getCompletions(const std::string& input);
};
```

#### CompletionProvider (接口)

```cpp
class CompletionProvider {
public:
    virtual std::vector<std::string> getCompletions(
        const CompletionContext& ctx) = 0;
    virtual int priority() const { return 0; }
};
```

**内置 Provider**:
- CommandCompleter (优先级: 100)
- VariableCompleter (优先级: 90)
- HistoryCompleter (优先级: 80)
- FileCompleter (优先级: 50)

### 6. 配置系统 (src/config/)

#### ConfigManager

```cpp
class ConfigManager {
public:
    bool loadConfig(const std::string& configPath);
    bool generateDefaultConfig(const std::string& configPath);
    std::optional<bool> getBool(const std::string& section,
                                const std::string& key) const;
    std::optional<int> getInt(const std::string& section,
                              const std::string& key) const;
    std::optional<std::string> getString(const std::string& section,
                                         const std::string& key) const;
    std::optional<std::string> getAlias(const std::string& name) const;
};
```

**配置结构**:
```cpp
struct ConfigValue {
    enum Type { BOOL, INT, STRING };
    Type type;
    std::string value;

    bool asBool() const;
    int asInt() const;
    std::string asString() const;
};
```

### 7. 语法高亮 (src/syntax/)

#### SyntaxHighlighter

```cpp
class SyntaxHighlighter {
public:
    explicit SyntaxHighlighter(const std::vector<std::string>& builtinCommands);
    std::string highlight(const std::string& input);
    bool isValidCommand(const std::string& command);
    void refreshPathCache();
};
```

**高亮规则**:
- 有效命令 → Green
- 无效命令 → Red
- 字符串 → Yellow
- 变量 → Blue
- 操作符 → Magenta

## 🔧 扩展开发

### 添加新的内建命令

1. 创建命令类:

```cpp
// src/builtin/mycommand.cpp
#include "builtin.h"

class MyCommand : public BuiltinCommand {
public:
    std::string getName() const override {
        return "mycommand";
    }

    std::string getHelp() const override {
        return "mycommand <args>   Do something";
    }

    BuiltinResult execute(const std::vector<std::string>& args,
                          BuiltinContext& context) override {
        BuiltinResult result;
        // 实现逻辑
        result.exitCode = 0;
        context.lastExitCode = result.exitCode;
        return result;
    }
};

extern "C" BuiltinCommand* createMyCommand() {
    return new MyCommand();
}
```

2. 在 builtin_manager.cpp 中注册:

```cpp
extern "C" {
    // ...
    BuiltinCommand* createMyCommand();
}

BuiltinManager::BuiltinManager() {
    // ...
    registerCommand(createMyCommand());
}
```

3. 在 CMakeLists.txt 中添加:

```cmake
add_executable(leizi
    # ...
    src/builtin/mycommand.cpp
)
```

### 添加新的补全提供者

```cpp
// src/completion/my_completer.h
class MyCompleter : public CompletionProvider {
public:
    std::vector<std::string> getCompletions(
        const CompletionContext& ctx) override {
        std::vector<std::string> completions;
        // 实现补全逻辑
        return completions;
    }

    int priority() const override {
        return 60;  // 优先级
    }
};
```

注册到 SmartCompleter:

```cpp
completer->addProvider(std::make_unique<MyCompleter>());
```

## 📊 数据结构

### Job

```cpp
enum class JobStatus { RUNNING, STOPPED, DONE };

struct Job {
    pid_t pid;
    int jobId;
    std::string command;
    JobStatus status;
    bool background;
    std::chrono::system_clock::time_point startTime;
};
```

### ParsedCommand

```cpp
struct Redirection {
    enum Type { INPUT, OUTPUT, APPEND, ERROR, ERROR_APPEND, BOTH };
    Type type;
    std::string filename;
};

struct ParsedCommand {
    std::vector<std::string> tokens;
    std::vector<Redirection> redirections;
    bool background;
};
```

## 🎨 颜色常量

```cpp
namespace Color {
    constexpr const char* RESET = "\033[0m";
    constexpr const char* RED = "\033[31m";
    constexpr const char* GREEN = "\033[32m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* BLUE = "\033[34m";
    constexpr const char* MAGENTA = "\033[35m";
    constexpr const char* CYAN = "\033[36m";
    constexpr const char* BOLD = "\033[1m";
    constexpr const char* DIM = "\033[2m";
}
```

## 🚀 性能优化

### Git 状态缓存

```cpp
// 分支名缓存 10 秒
static constexpr int BRANCH_CACHE_SECONDS = 10;

// 状态缓存 2 秒
static constexpr int STATUS_CACHE_SECONDS = 2;
```

### PATH 命令缓存

SyntaxHighlighter 在初始化时扫描 PATH 并缓存所有可执行文件：

```cpp
void cachePathCommands() {
    // 扫描 PATH
    // 检查可执行权限
    // 存储到 pathCommands_ set
}
```

## 📝 最佳实践

1. **内存管理**: 使用 RAII 和智能指针
2. **错误处理**: 优先使用异常
3. **命名规范**:
   - 类: PascalCase
   - 函数: camelCase
   - 变量: camelCase
   - 常量: UPPER_CASE
4. **注释**: 使用 Doxygen 风格

---

**版本**: 1.3.0
**最后更新**: 2025-10-03
