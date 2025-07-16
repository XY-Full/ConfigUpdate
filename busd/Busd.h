#pragma once
#include "Channel.h"
#include "NetPack.h"
#include <unordered_map>
#include <functional>
#include <thread>
#include "iostream"

class Busd 
{
public:
    using Handler = std::function<void(int64_t conn_id, const NetPack&)>;

    void register_handler(int32_t msg_id, Handler handler) 
    {
        handlers_[msg_id] = std::move(handler);
    }

    void set_channels(
        Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* in,
        Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* out
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
        out_channel_->push({conn_id, std::make_shared<NetPack>(pack)});
    }

private:
    void process_messages() 
    {
        while (running_) 
        {
            std::pair<int64_t, std::shared_ptr<NetPack>> item;
            if (!in_channel_) 
            {
                std::cerr << "in_channel is nullptr!!" << std::endl;
                return;
            }
            if (in_channel_->try_pop(item)) 
            {
                auto conn_id = item.first;
                auto pack = item.second;
                auto body = pack->msg;

                std::cout << "busd recv body : " << body << std::endl;

                try 
                {
		            std::cout << "msg_id : " << pack->msg_id << std::endl;
                    auto it = handlers_.find(pack->msg_id);
                    if (it != handlers_.end()) 
                    {
                        std::cout << "trigger handlers!" << std::endl;
                        it->second(conn_id, *pack);
                    } 
		            else 
                    {
			        std::cout << "msg_id :" << pack->msg_id << " never be registed!" << std::endl;
                        // 未注册 msg_id 的处理器
                        // 可记录日志或忽略
                    }
                } 
                catch (const std::exception& e) 
                {
		            std::cout << "deserialize error to NetPack!" << std::endl;
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
    Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* in_channel_ = nullptr;
    Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* out_channel_ = nullptr;
    std::thread worker_;
    std::atomic<bool> running_ = false;
};
