#pragma once
#include "Channel.h"
#include "NetPack.h"
#include <functional>
#include <thread>
#include "Timer.h"
#include "Log.h"
#include "google/protobuf/message.h"

class Busd 
{
public:
    using Handler = std::function<void(const NetPack&)>;

    Busd(Timer* loop,
        Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* in,
        Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* out) : running_(false), loop_(loop), in_channel_(in), out_channel_(out) {}

    void registerHandler(int32_t msg_id, Handler handler) 
    {
        handlers_[msg_id] = std::move(handler);
    }

    void start() 
    {
        running_ = true;
        worker_ = std::thread(&Busd::processMessages, this);
    }

    void stop() 
    {
        running_ = false;
        if (worker_.joinable()) worker_.join();
    }

    void sendToClient(int64_t conn_id, const NetPack& pack) 
    {
        out_channel_->push({conn_id, std::make_shared<NetPack>(pack)});
    }

    void replyToClient(const NetPack& request, const google::protobuf::Message& msg)
    {
        auto responsePack = std::make_shared<NetPack>(request, &msg);
        out_channel_->push({request.conn_id, responsePack});
    }

private:
    void processMessages() 
    {
        while (running_) 
        {
            std::pair<int64_t, std::shared_ptr<NetPack>> item;
            if (!in_channel_) 
            {
                ELOG << "in_channel is nullptr!!";
                return;
            }
            if (in_channel_->try_pop(item)) 
            {
                auto conn_id = item.first;
                auto pack = item.second;
                auto body = pack->msg;

                DLOG << "busd recv body : " << body;

                try 
                {
		            std::cout << "msg_id : " << pack->msg_id << std::endl;
                    auto it = handlers_.find(pack->msg_id);
                    if (it != handlers_.end())
                    {
                        ILOG << "trigger handlers!";
                        it->second(*pack);
                    }
                    else
                    {
			            ELOG << "msg_id :" << pack->msg_id << " never be registed!";
                        // 未注册 msg_id 的处理器
                        // 可记录日志或忽略
                    }
                } 
                catch (const std::exception& e) 
                {
		            ELOG << "deserialize error to NetPack!";
                    // 反序列化错误处理（可选日志）
                }
            } 
            else 
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }

    std::unordered_map<int32_t, Handler>    handlers_;
    Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* in_channel_ = nullptr;
    Channel<std::pair<int64_t, std::shared_ptr<NetPack>>>* out_channel_ = nullptr;
    Timer*                                  loop_;
    std::thread                             worker_;
    std::atomic<bool>                       running_ = false;
};
