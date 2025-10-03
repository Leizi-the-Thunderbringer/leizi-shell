#pragma once

#include <string>
#include <vector>
#include <memory>
#include "utils/variables.h"

namespace leizi {

// 补全上下文信息
struct CompletionContext {
    std::vector<std::string> tokens;      // 当前命令的所有token
    std::string currentToken;             // 当前正在补全的token
    size_t tokenIndex;                    // 当前token的索引
    bool isFirstToken;                    // 是否是第一个token (命令名)
    std::string fullInput;                // 完整输入
};

// 补全提供者基类
class CompletionProvider {
public:
    virtual ~CompletionProvider() = default;
    virtual std::vector<std::string> getCompletions(const CompletionContext& ctx) = 0;
    virtual int priority() const { return 0; }  // 优先级，数字越大优先级越高
};

// 命令补全 (builtin + PATH)
class CommandCompleter : public CompletionProvider {
public:
    CommandCompleter(const std::vector<std::string>& builtins);
    std::vector<std::string> getCompletions(const CompletionContext& ctx) override;
    int priority() const override { return 100; }

private:
    std::vector<std::string> builtinCommands;
    std::vector<std::string> getPathCommands(const std::string& prefix) const;
};

// 文件/目录补全
class FileCompleter : public CompletionProvider {
public:
    std::vector<std::string> getCompletions(const CompletionContext& ctx) override;
    int priority() const override { return 50; }

private:
    std::string expandTilde(const std::string& path) const;
};

// 变量名补全 ($VAR)
class VariableCompleter : public CompletionProvider {
public:
    explicit VariableCompleter(const VariableManager& vm);
    std::vector<std::string> getCompletions(const CompletionContext& ctx) override;
    int priority() const override { return 90; }

private:
    const VariableManager& variables;
};

// 历史命令补全
class HistoryCompleter : public CompletionProvider {
public:
    explicit HistoryCompleter(const std::vector<std::string>& history);
    std::vector<std::string> getCompletions(const CompletionContext& ctx) override;
    int priority() const override { return 80; }

private:
    const std::vector<std::string>& commandHistory;
};

// 智能补全管理器
class SmartCompleter {
public:
    SmartCompleter();

    void addProvider(std::unique_ptr<CompletionProvider> provider);
    std::vector<std::string> getCompletions(const std::string& input);

private:
    std::vector<std::unique_ptr<CompletionProvider>> providers;
    CompletionContext analyzeInput(const std::string& input) const;
};

} // namespace leizi
