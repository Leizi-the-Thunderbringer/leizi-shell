#ifndef LEIZI_SYNTAX_HIGHLIGHTER_H
#define LEIZI_SYNTAX_HIGHLIGHTER_H

#include <string>
#include <vector>
#include <set>
#include <memory>

namespace leizi {

/**
 * @brief 语法高亮器类
 *
 * 为命令行输入提供实时语法高亮
 * 支持命令、变量、字符串、操作符等的着色
 */
class SyntaxHighlighter {
public:
    /**
     * @brief 构造函数
     * @param builtinCommands 内建命令列表
     */
    explicit SyntaxHighlighter(const std::vector<std::string>& builtinCommands);

    /**
     * @brief 对输入文本应用语法高亮
     * @param input 原始输入字符串
     * @return 带ANSI颜色码的高亮字符串
     */
    std::string highlight(const std::string& input);

    /**
     * @brief 检查命令是否存在（在PATH或builtin中）
     * @param command 命令名
     * @return true 如果命令存在
     */
    bool isValidCommand(const std::string& command);

    /**
     * @brief 更新PATH缓存（当PATH环境变量改变时调用）
     */
    void refreshPathCache();

private:
    std::set<std::string> builtinCommands_;
    std::set<std::string> pathCommands_;

    /**
     * @brief 扫描PATH环境变量，缓存可执行文件
     */
    void cachePathCommands();

    /**
     * @brief 检测字符串边界（单引号或双引号）
     * @param input 输入字符串
     * @param pos 当前位置
     * @param quote 引号字符
     * @return 字符串结束位置
     */
    size_t findStringEnd(const std::string& input, size_t pos, char quote);

    /**
     * @brief 判断是否为操作符字符
     */
    bool isOperator(char c);
};

} // namespace leizi

#endif // LEIZI_SYNTAX_HIGHLIGHTER_H
