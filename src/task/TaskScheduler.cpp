#include "TaskScheduler.h"
#include "../core/Logger.h"

void TaskScheduler::start(u32 threadCount) {
    m_stopping = false;
    m_workers.reserve(threadCount);
    for (u32 i = 0; i < threadCount; i++) {
        m_workers.emplace_back([this]() { workerLoop(); });
    }
    Logger::instance().info("Task scheduler started with " + std::to_string(threadCount) + " threads");
}

void TaskScheduler::stop() {
    m_stopping = true;
    m_condition.notify_all();
    for (auto& t : m_workers) {
        if (t.joinable()) t.join();
    }
    m_workers.clear();
}

void TaskScheduler::workerLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock lock(m_mutex);
            m_condition.wait(lock, [this]() {
                return m_stopping || !m_tasks.empty();
            });
            if (m_stopping && m_tasks.empty()) return;
            task = std::move(m_tasks.front());
            m_tasks.pop();
        }
        task();
    }
}
