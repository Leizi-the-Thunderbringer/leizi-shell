#include "core/job_control.h"

#include <algorithm>
#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

int JobControl::addJob(pid_t pid, const std::string& command, bool background) {
    int jobId = m_nextJobId++;
    m_jobs.emplace_back(jobId, pid, command, background);

    if (background) {
        std::cout << "[" << jobId << "] " << pid << std::endl;
    }

    return jobId;
}

void JobControl::updateJobStatus() {
    for (auto& job : m_jobs) {
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
    m_jobs.erase(std::remove_if(m_jobs.begin(), m_jobs.end(),
        [](const Job& job) { return job.status == JobStatus::DONE; }),
        m_jobs.end());
}

void JobControl::listJobs() const {
    if (m_jobs.empty()) {
        std::cout << "No jobs running" << std::endl;
        return;
    }

    for (const auto& job : m_jobs) {
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

bool JobControl::foregroundJob(int jobId) {
    auto it = std::find_if(m_jobs.begin(), m_jobs.end(),
        [jobId](const Job& job) { return job.jobId == jobId; });

    if (it == m_jobs.end()) {
        std::cerr << "leizi: fg: job " << jobId << " not found" << std::endl;
        return false;
    }

    if (it->status == JobStatus::DONE) {
        std::cerr << "leizi: fg: job has terminated" << std::endl;
        m_jobs.erase(it);
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

    // 等待作业完成
    int status;
    waitpid(it->pid, &status, WUNTRACED);

    if (WIFEXITED(status) || WIFSIGNALED(status)) {
        m_jobs.erase(it);
    } else if (WIFSTOPPED(status)) {
        it->status = JobStatus::STOPPED;
        std::cout << "[" << it->jobId << "]+ Stopped\t"
                 << it->command << std::endl;
    }

    return true;
}

bool JobControl::backgroundJob(int jobId) {
    auto it = std::find_if(m_jobs.begin(), m_jobs.end(),
        [jobId](const Job& job) { return job.jobId == jobId; });

    if (it == m_jobs.end()) {
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

Job* JobControl::findJob(int jobId) {
    auto it = std::find_if(m_jobs.begin(), m_jobs.end(),
        [jobId](const Job& job) { return job.jobId == jobId; });

    return it != m_jobs.end() ? &(*it) : nullptr;
}

int JobControl::getLatestJobId() const {
    return m_jobs.empty() ? -1 : m_jobs.back().jobId;
}
