#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <condition_variable>

class Timer {
public:
    using TaskId = uint64_t;
    using Callback = std::function<void()>;

    Timer();
    ~Timer();

    // 添加一次性定时任务，delaySeconds 秒后执行一次 callback
    TaskId runAfter(double delaySeconds, Callback cb);

    // 添加周期性定时任务，每 intervalSeconds 秒执行 callback
    TaskId runEvery(double intervalSeconds, Callback cb);

    // 取消任务
    void cancel(TaskId id);

    // 停止所有任务
    void shutdown();

private:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;

    struct Task 
    {
        TimePoint nextRun;
        Duration interval;  // 0 表示一次性任务
        Callback cb;
    };

    void workerThread();

    std::atomic<bool> running_;
    std::thread worker_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::map<TaskId, Task> tasks_;
    std::atomic<TaskId> nextId_;
};
