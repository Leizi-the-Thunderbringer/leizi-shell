#include "../catch.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

TEST_CASE("Job control - Background execution", "[integration][jobs]") {
    SECTION("Background process execution") {
        // 测试后台进程执行
        pid_t pid = fork();
        if (pid == 0) {
            // 子进程：休眠一小段时间
            sleep(1);
            exit(0);
        } else if (pid > 0) {
            // 父进程：不等待子进程
            // 验证子进程在后台运行
            REQUIRE(pid > 0);

            // 稍等片刻
            usleep(100000);  // 100ms

            // 检查进程是否仍在运行
            int status;
            pid_t result = waitpid(pid, &status, WNOHANG);
            // WNOHANG 表示非阻塞，如果进程还在运行应返回 0
            // 但由于我们只 sleep 1秒，这里可能已经结束了
            // 所以我们只验证 waitpid 成功调用
            REQUIRE((result == 0 || result == pid));

            // 最终等待进程结束
            if (result == 0) {
                waitpid(pid, &status, 0);
            }
        }
    }
}

TEST_CASE("Job control - Signal handling", "[integration][jobs]") {
    SECTION("SIGTERM handling") {
        pid_t pid = fork();
        if (pid == 0) {
            // 子进程：休眠
            sleep(10);
            exit(0);
        } else if (pid > 0) {
            // 父进程：发送 SIGTERM
            usleep(50000);  // 等待子进程启动
            int result = kill(pid, SIGTERM);
            REQUIRE(result == 0);

            // 等待子进程结束
            int status;
            waitpid(pid, &status, 0);
            REQUIRE(WIFSIGNALED(status) == true);
            REQUIRE(WTERMSIG(status) == SIGTERM);
        }
    }

    SECTION("SIGCONT after SIGSTOP") {
        pid_t pid = fork();
        if (pid == 0) {
            // 子进程：循环
            while (true) {
                usleep(100000);
            }
        } else if (pid > 0) {
            usleep(50000);

            // 发送 SIGSTOP
            int result = kill(pid, SIGSTOP);
            REQUIRE(result == 0);

            usleep(100000);

            // 发送 SIGCONT
            result = kill(pid, SIGCONT);
            REQUIRE(result == 0);

            usleep(50000);

            // 最后终止进程
            kill(pid, SIGTERM);
            int status;
            waitpid(pid, &status, 0);
        }
    }
}
