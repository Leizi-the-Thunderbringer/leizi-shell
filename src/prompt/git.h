#ifndef LEIZI_PROMPT_GIT_H
#define LEIZI_PROMPT_GIT_H

#include <string>

/**
 * @brief Git 集成功能类
 *
 * 负责获取 Git 仓库信息，包括分支名和文件状态
 */
class GitIntegration {
public:
    /**
     * @brief 检查当前目录是否为 Git 仓库
     * @return true 如果是 Git 仓库
     */
    static bool isGitRepository();

    /**
     * @brief 获取当前 Git 分支名
     * @return 分支名，若不在仓库中则返回空字符串
     */
    static std::string getBranch();

    /**
     * @brief 获取 Git 工作区状态（修改、添加、删除的文件数）
     * @return 格式化的状态字符串，包含颜色码
     */
    static std::string getStatus();

private:
    /**
     * @brief 执行 shell 命令并返回输出
     * @param command 要执行的命令
     * @return 命令输出（去除尾部换行符）
     */
    static std::string executeCommand(const std::string& command);
};

#endif // LEIZI_PROMPT_GIT_H
