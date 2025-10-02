#ifndef LEIZI_CORE_JOB_CONTROL_H
#define LEIZI_CORE_JOB_CONTROL_H

#include <chrono>
#include <string>
#include <sys/types.h>
#include <vector>

/**
 * @brief 作业状态枚举
 */
enum class JobStatus {
    RUNNING,    // 正在运行
    STOPPED,    // 已停止（Ctrl+Z）
    DONE        // 已完成
};

/**
 * @brief 作业信息结构
 */
struct Job {
    int jobId;                      // 作业ID
    pid_t pid;                       // 进程ID
    std::string command;             // 命令字符串
    JobStatus status;                // 作业状态
    bool background;                 // 是否后台运行
    std::chrono::system_clock::time_point startTime;  // 启动时间

    Job(int id, pid_t p, const std::string& cmd, bool bg = false)
        : jobId(id), pid(p), command(cmd), status(JobStatus::RUNNING),
          background(bg), startTime(std::chrono::system_clock::now()) {}
};

/**
 * @brief 作业控制管理器
 *
 * 负责管理后台作业、前台作业切换等功能
 */
class JobControl {
public:
    /**
     * @brief 添加新作业
     * @param pid 进程ID
     * @param command 命令字符串
     * @param background 是否后台运行
     * @return 作业ID
     */
    int addJob(pid_t pid, const std::string& command, bool background);

    /**
     * @brief 更新所有作业的状态
     */
    void updateJobStatus();

    /**
     * @brief 列出所有作业
     */
    void listJobs() const;

    /**
     * @brief 将作业置于前台
     * @param jobId 作业ID
     * @return 成功返回true
     */
    bool foregroundJob(int jobId);

    /**
     * @brief 将作业置于后台
     * @param jobId 作业ID
     * @return 成功返回true
     */
    bool backgroundJob(int jobId);

    /**
     * @brief 根据作业ID查找作业
     * @param jobId 作业ID
     * @return 作业指针，未找到返回nullptr
     */
    Job* findJob(int jobId);

    /**
     * @brief 获取所有作业
     */
    const std::vector<Job>& getJobs() const { return m_jobs; }

    /**
     * @brief 获取最近的作业ID
     */
    int getLatestJobId() const;

private:
    std::vector<Job> m_jobs;
    int m_nextJobId = 1;
};

#endif // LEIZI_CORE_JOB_CONTROL_H
