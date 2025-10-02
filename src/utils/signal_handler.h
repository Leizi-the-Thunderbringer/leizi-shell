#ifndef LEIZI_UTILS_SIGNAL_HANDLER_H
#define LEIZI_UTILS_SIGNAL_HANDLER_H

#include <signal.h>
#include <sys/types.h>

/**
 * @brief 信号处理器类
 *
 * 管理 shell 的信号处理逻辑
 */
class SignalHandler {
public:
    /**
     * @brief 初始化信号处理器
     */
    static void initialize();

    /**
     * @brief 检查是否收到中断信号
     */
    static bool wasInterrupted();

    /**
     * @brief 重置中断标志
     */
    static void resetInterrupted();

    /**
     * @brief 设置前台进程PID
     * @param pid 前台进程ID
     */
    static void setForegroundPid(pid_t pid);

    /**
     * @brief 获取前台进程PID
     */
    static pid_t getForegroundPid();

private:
    static void signalHandler(int signal);

    static bool s_interrupted;
    static pid_t s_foregroundPid;
};

#endif // LEIZI_UTILS_SIGNAL_HANDLER_H
