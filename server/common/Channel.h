#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>

// 线程安全的Channel模板类
template <typename T>
class Channel 
{
public:
    void push(const T& item) 
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
        cond_.notify_one();
    }

    bool pop(T& item) 
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return !queue_.empty(); });
        item = queue_.front();
        queue_.pop();
        return true;
    }

    T pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return !queue_.empty(); });
        T item = queue_.front();
        queue_.pop();
        return item;
    }

    bool try_pop(T& item) 
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) return false;
        item = queue_.front();
        queue_.pop();
        return true;
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};