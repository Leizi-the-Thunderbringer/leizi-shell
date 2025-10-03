#ifndef LEIZI_PROMPT_GIT_H
#define LEIZI_PROMPT_GIT_H

#include <string>
#include <chrono>
#include <optional>

/**
 * @brief Git 集成功能类
 *
 * 负责获取 Git 仓库信息，包括分支名和文件状态
 * 使用缓存机制优化性能，减少 git 命令调用
 */
class GitIntegration {
public:
    /**
     * @brief 检查当前目录是否为 Git 仓库
     * @return true 如果是 Git 仓库
     */
    static bool isGitRepository();

    /**
     * @brief 获取当前 Git 分支名（带缓存）
     * @param forceRefresh 是否强制刷新缓存
     * @return 分支名，若不在仓库中则返回空字符串
     */
    static std::string getBranch(bool forceRefresh = false);

    /**
     * @brief 获取 Git 工作区状态（修改、添加、删除的文件数，带缓存）
     * @param forceRefresh 是否强制刷新缓存
     * @return 格式化的状态字符串，包含颜色码
     */
    static std::string getStatus(bool forceRefresh = false);

    /**
     * @brief 清除所有缓存
     */
    static void clearCache();

private:
    /**
     * @brief 执行 shell 命令并返回输出
     * @param command 要执行的命令
     * @return 命令输出（去除尾部换行符）
     */
    static std::string executeCommand(const std::string& command);

    // 缓存相关
    struct Cache {
        std::string branch;
        std::string status;
        std::chrono::steady_clock::time_point branchTime;
        std::chrono::steady_clock::time_point statusTime;
        std::string lastWorkingDir;
    };

    static Cache cache;
    static constexpr int BRANCH_CACHE_SECONDS = 10; // 分支名缓存10秒
    static constexpr int STATUS_CACHE_SECONDS = 2;  // 状态缓存2秒
};

#endif // LEIZI_PROMPT_GIT_H
