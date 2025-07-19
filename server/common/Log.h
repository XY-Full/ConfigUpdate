#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <mutex>
#include <thread>

class Logger 
{
public:
    enum Level { DEBUG, INFO, WARN, ERROR };

    Logger(Level level, const char* file, int line)
        : level_(level) 
    {
        // 时间格式化
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        stream_ << "[" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S")
                << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";

        stream_ << "[" << levelToString(level) << "] ";
        stream_ << "[" << std::this_thread::get_id() << "] ";
        stream_ << "[" << file << ":" << line << "] ";
    }

    ~Logger() 
    {
        stream_ << std::endl;
        std::lock_guard<std::mutex> lock(getMutex());
        std::cout << stream_.str();
        std::cout.flush();
    }

    template<typename T>
    Logger& operator<<(const T& val) 
    {
        stream_ << val;
        return *this;
    }

private:
    static std::mutex& getMutex() 
    {
        static std::mutex mtx;
        return mtx;
    }

    static const char* levelToString(Level level) 
    {
        switch (level) 
        {
            case DEBUG: return "DEBUG"; 
            case INFO:  return "INFO"; 
            case WARN:  return "WARN"; 
            case ERROR: return "ERROR"; 
            default:    return "UNKNOWN";
        }
    }

    Level level_;
    std::ostringstream stream_;
};

// 宏定义方便使用，自动传入文件名和行号
#define DLOG Logger(Logger::DEBUG, __FILE__, __LINE__)
#define ILOG Logger(Logger::INFO,  __FILE__, __LINE__)
#define WLOG Logger(Logger::WARN,  __FILE__, __LINE__)
#define ELOG Logger(Logger::ERROR, __FILE__, __LINE__)
