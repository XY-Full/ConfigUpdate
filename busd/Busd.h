#pragma once
#include "Channel.h"
#include "NetPack.h"
#include <unordered_map>
#include <functional>
#include <thread>

class Busd 
{
public:
    using Handler = std::function<void(int64_t conn_id, const NetPack&)>;

    void register_handler(int32_t msg_id, Handler handler) 
    {
        handlers_[msg_id] = std::move(handler);
    }

    void set_channels(
        Channel<std::pair<int64_t, std::string>>* in,
        Channel<std::pair<int64_t, std::string>>* out
    ) 
    {
        in_channel_ = in;
        out_channel_ = out;
    }

    void start() 
    {
        running_ = true;
        worker_ = std::thread(&Busd::process_messages, this);
    }

    void stop() 
    {
        running_ = false;
        if (worker_.joinable()) worker_.join();
    }

    void send_to_client(int64_t conn_id, const NetPack& pack) 
    {
        out_channel_->push({conn_id, pack.serialize()});
    }

private:
    void process_messages() 
    {
        while (running_) 
        {
            std::pair<int64_t, std::string> item;
            if (in_channel_->try_pop(item)) 
            {
                int64_t conn_id = item.first;
                const std::string& raw = item.second;

                try 
                {
                    NetPack pack = NetPack::deserialize(raw);
                    auto it = handlers_.find(pack.msg_id);
                    if (it != handlers_.end()) 
                    {
                        it->second(conn_id, pack);
                    } else 
                    {
                        // 未注册 msg_id 的处理器
                        // 可记录日志或忽略
                    }
                } 
                catch (const std::exception& e) 
                {
                    // 反序列化错误处理（可选日志）
                }
            } 
            else 
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }

    std::unordered_map<int32_t, Handler> handlers_;
    Channel<std::pair<int64_t, std::string>>* in_channel_ = nullptr;
    Channel<std::pair<int64_t, std::string>>* out_channel_ = nullptr;
    std::thread worker_;
    std::atomic<bool> running_ = false;
};
