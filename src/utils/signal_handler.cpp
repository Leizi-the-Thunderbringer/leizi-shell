#include "utils/signal_handler.h"

#include <iostream>

// 静态成员初始化
bool SignalHandler::s_interrupted = false;
pid_t SignalHandler::s_foregroundPid = -1;

void SignalHandler::initialize() {
    // 设置信号处理器
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sa, nullptr);   // Ctrl+C
    sigaction(SIGTSTP, &sa, nullptr);  // Ctrl+Z
}

bool SignalHandler::wasInterrupted() {
    return s_interrupted;
}

void SignalHandler::resetInterrupted() {
    s_interrupted = false;
}

void SignalHandler::setForegroundPid(pid_t pid) {
    s_foregroundPid = pid;
}

pid_t SignalHandler::getForegroundPid() {
    return s_foregroundPid;
}

void SignalHandler::signalHandler(int signal) {
    if (signal == SIGINT) {
        s_interrupted = true;
        std::cout << "\n";
    } else if (signal == SIGTSTP) {
        // Ctrl+Z - 暂停前台进程
        if (s_foregroundPid > 0) {
            kill(s_foregroundPid, SIGTSTP);
        }
    }
}
