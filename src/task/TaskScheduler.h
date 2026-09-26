#pragma once
#include "../core/Types.h"
#include <functional>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>

class TaskScheduler {
public:
    TaskScheduler() = default;
    ~TaskScheduler() { stop(); }

    TaskScheduler(const TaskScheduler&) = delete;
    TaskScheduler& operator=(const TaskScheduler&) = delete;

    void start(u32 threadCount);
    void stop();

    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<decltype(f(args...))> {
        using ReturnType = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        auto result = task->get_future();
        {
            std::lock_guard lock(m_mutex);
            if (m_stopping) throw std::runtime_error("enqueue on stopped scheduler");
            m_tasks.emplace([task]() { (*task)(); });
        }
        m_condition.notify_one();
        return result;
    }

    u32 threadCount() const { return (u32)m_workers.size(); }
    u32 pendingTasks() const {
        std::lock_guard lock(m_mutex);
        return (u32)m_tasks.size();
    }

private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    std::atomic<bool> m_stopping{false};

    void workerLoop();
};
