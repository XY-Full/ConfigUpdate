#include "Timer.h"

Timer::Timer()
    : running_(true), nextId_(1)
{
    worker_ = std::thread([this] { workerThread(); });
}

Timer::~Timer()
{
    shutdown();
}

Timer::TaskId Timer::runAfter(double delaySeconds, Callback cb)
{
    return runEvery(delaySeconds, std::move(cb));  // 设置为周期，再在 worker 中识别一次性
}

Timer::TaskId Timer::runEvery(double intervalSeconds, Callback cb)
{
    TaskId id = nextId_.fetch_add(1);

    auto interval = std::chrono::duration_cast<Duration>(
        std::chrono::duration<double>(intervalSeconds)
    );

    Task task;
    task.interval = interval;
    task.nextRun = Clock::now() + interval;
    task.cb = std::move(cb);

    {
        std::lock_guard lock(mutex_);
        tasks_.emplace(id, std::move(task));
    }
    cv_.notify_one();
    return id;
}

void Timer::cancel(TaskId id)
{
    std::lock_guard lock(mutex_);
    tasks_.erase(id);
    cv_.notify_one();
}

void Timer::shutdown()
{
    if (!running_) return;
    running_ = false;
    cv_.notify_one();
    if (worker_.joinable()) worker_.join();
}

void Timer::workerThread()
{
    std::unique_lock lock(mutex_);
    while (running_) 
    {
        if (tasks_.empty()) 
        {
            // 没有任务时，等待新任务或 shutdown
            cv_.wait(lock, [this] { return !running_ || !tasks_.empty(); });
        } 
        else 
        {
            // 找到下一个最近要执行的任务
            auto nextIt = std::min_element(
                tasks_.begin(), tasks_.end(),
                [](auto &a, auto &b) {
                    return a.second.nextRun < b.second.nextRun;
                });
            auto now = Clock::now();
            if (now >= nextIt->second.nextRun) 
            {
                // 到点，执行
                auto task = nextIt->second;
                TaskId id = nextIt->first;
                // 如果是一次性任务（interval == 0），先删除
                tasks_.erase(nextIt);
                lock.unlock();
                try 
                {
                    task.cb();
                } 
                catch (...) 
                {
                    // 忽略任务内部异常
                }
                lock.lock();
                // 如果是周期任务，重新入队
                if (task.interval.count() > 0 && running_) 
                {
                    task.nextRun = Clock::now() + task.interval;
                    tasks_.emplace(id, std::move(task));
                }
            } 
            else 
            {
                // 在下一个任务触发前等待
                cv_.wait_until(lock, nextIt->second.nextRun);
            }
        }
    }
    // 退出前清理所有任务
    tasks_.clear();
}
